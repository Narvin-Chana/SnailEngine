#pragma once
#include <comdef.h>
#include <string>
#include <optional>
#include "Util/SnailException.h"
#include <vector>

#include "Core/Camera/Camera.h"

namespace Snail
{
class GrassGenerator;
class Entity;
class GrassGenerator;

inline std::string WStringToString(const std::wstring& wstr)
{
    std::string s;
    s.resize(wstr.size());
    int usedDefaultChar = 0;
    WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), static_cast<int>(wstr.size()), s.data(), static_cast<int>(s.size()), "#", &usedDefaultChar);
    return s;
}

inline std::wstring StringToWString(const std::string& str)
{
    std::wstring s;
    s.resize(str.size());
    MultiByteToWideChar(CP_ACP, 0, str.c_str(), static_cast<int>(str.size()), s.data(), static_cast<int>(s.size()));
    return s;
}

template <class OriginalErrorType, class OverrideErrorType>
struct DXError : SnailException
{
    std::wstring errorMessage;
    OriginalErrorType originalError;
    std::optional<OverrideErrorType> overrideError;

    DXError(OriginalErrorType ogError, OverrideErrorType ovError, const int line = 0, const char* file = "")
        : SnailException(line, file)
        , originalError{ogError}
        , overrideError{ovError}
    {
        LOGF("DX ERROR: {:X} {}", ogError, ovError);
        const _com_error err(ogError);
        errorMessage = std::wstring(err.ErrorMessage()) + L"\nLine: " + std::to_wstring(line) + L"\nFile: " + StringToWString(std::string{file});
    }

    DXError(OriginalErrorType ogError, OverrideErrorType ovError, const std::string& errorMessage, const int line = 0, const char* file = "")
        : SnailException(line, file)
        , errorMessage{StringToWString(errorMessage)}
        , originalError{ogError}
        , overrideError{ovError}
    {
        LOGF("DX ERROR: {:X} {}: \"{}\"", ogError, ovError, errorMessage);
        this->errorMessage = this->errorMessage + L"\nLine: " + std::to_wstring(line) + L"\nFile: " + StringToWString(std::string{file});
    }

    DXError(OriginalErrorType ogError, const std::string& errorMessage, const int line = 0, const char* file = "")
        : SnailException(line, file)
        , errorMessage{StringToWString(errorMessage)}
        , originalError{ogError}
        , overrideError{}
    {
        LOGF("DX ERROR: {:X}: \"{}\"", ogError, errorMessage);
        this->errorMessage = this->errorMessage + L"\nLine: " + std::to_wstring(line) + L"\nFile: " + StringToWString(std::string{file});
    }
};

#define DX_CALL(...) DXThrowIfError(__VA_ARGS__, __LINE__, __FILE__)

#define DX_COMPILE_SHADER(blob, ...) DXThrowIfBlobError(__VA_ARGS__, __LINE__, __FILE__, blob)

template <class Type1, class Type2>
void DXThrowIfBlobError(const Type1& result, const Type2& errCode, int line, const char* file, ID3DBlob* blob)
{
    if (FAILED(result))
    {
        const char* errorMessage = "No more information was provided. Please check the logs for more info.";
        if (blob)
        {
            errorMessage = static_cast<char*>(blob->GetBufferPointer());
        }
        throw DXError(result, errCode, errorMessage, line, file);
    }
}

template <class Type>
void DXThrowIfError(const Type& result, const std::string& errorMessage, int line, const char* file)
{
    if (FAILED(result))
    {
        throw DXError(result, errorMessage, line, file);
    }
}

template <class Type1, class Type2>
void DXThrowIfError(const Type1& result, const Type2& errCode, int line, const char* file)
{
    if (FAILED(result))
    {
        throw DXError(result, errCode, line, file);
    }
}

template <class Type1, class Type2>
void DXThrowIfError(const Type1& result, const Type2& errCode, const std::string& errorMessage, int line, const char* file)
{
    if (FAILED(result))
    {
        throw DXError(result, errCode, errorMessage, line, file);
    }
}

template <class Type>
void DXValidate(const void* pointer, const Type& errCode) { if (pointer == nullptr) { throw errCode; } }

#define DX_RELEASE(ptr) DXRelease(ptr, __FILE__, __LINE__)

template <class Type>
void DXRelease(Type* pointer, const char* file = "", int line = 0) noexcept
{
    UNREFERENCED_PARAMETER(file);
    UNREFERENCED_PARAMETER(line);
    if (pointer != nullptr)
    {
#ifdef _DEBUG
        auto refsLeft = pointer->Release();
        if (refsLeft == 0)
        {
            //LOGF("DX Release: 0x{:016X} released completely", reinterpret_cast<uint64_t>(pointer), refsLeft);
        }
        else
        {
            //LOGF("DX Release: 0x{:016X} ({} refs left)\nfile: {}\nline: {}", reinterpret_cast<uint64_t>(pointer), refsLeft, file, line);
        }
#else
        pointer->Release();
#endif
        pointer = nullptr;
    }
}

template <typename T>
int GetDXRefCount(T* object)
{
    object->AddRef();
    return object->Release();
}

inline unsigned int RaiseToNextMultipleOf(unsigned int val, unsigned int multiple)
{
    if (val == 0)
        return 0;

    return val + (multiple - val % multiple);
}

// Compiler doesnt like me padding a lot
#pragma warning (disable : 4324)
#pragma warning (disable : 4201)

static constexpr auto DX_ALIGNMENT = 16;
#define DX_ALIGN alignas (DX_ALIGNMENT)

static constexpr auto DX_ALIGNMENT_BOOL = 4;
#define DX_ALIGN_BOOL alignas (DX_ALIGNMENT_BOOL)

inline Vector3 GetTriangleNormal(const Vector3& p1, const Vector3& p2, const Vector3& p3)
{
    const auto v1 = p2 - p1;
    const auto v2 = p2 - p3;
    auto v3 = v1.Cross(v2);
    v3.Normalize();
    return v3;
}

std::vector<Vector3> GetFrustumCornersWorldSpace(const Matrix& viewProj);

bool IsInFrustum(const DirectX::BoundingFrustum& frustum, const DirectX::BoundingBox& boundingBox);
bool IsInFrustum(const DirectX::BoundingFrustum& frustum, const DirectX::BoundingOrientedBox& boundingOrientedBox);
bool IsInFrustum(const DirectX::BoundingFrustum& frustum, Entity& entity);
bool IsGrassPatchInFrustum(const DirectX::BoundingFrustum& frustum, const GrassGenerator& grassPatch);

bool IsGrassPatchInFrustum(const DirectX::BoundingFrustum& frustum, const GrassGenerator& grassPatch);

DirectX::BoundingFrustum GetFrustumFromCamera(const Camera* projectionCamera);

};
