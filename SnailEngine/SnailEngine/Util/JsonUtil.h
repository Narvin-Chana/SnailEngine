#pragma once

#include <DirectXMath.h>
#include <json.hpp>

namespace DirectX
{

inline void to_json(nlohmann::json& j, const XMFLOAT3& p)
{
    j = nlohmann::json{p.x, p.y, p.z};
}

inline void from_json(const nlohmann::json& j, XMFLOAT3& p)
{
    const auto vals = j.get<std::array<float, 3>>();
    p.x = vals[0];
    p.y = vals[1];
    p.z = vals[2];
}

inline void from_json(const nlohmann::json& j, XMFLOAT2& p)
{
    const auto vals = j.get<std::array<float, 2>>();
    p.x = vals[0];
    p.y = vals[1];
}

inline void to_json(nlohmann::json& j, const XMFLOAT4& p)
{
    j = nlohmann::json{ p.x, p.y, p.z, p.w };
}

inline void from_json(const nlohmann::json& j, XMFLOAT4& p)
{
    const auto vals = j.get<std::array<float, 4>>();
    p.x = vals[0];
    p.y = vals[1];
    p.z = vals[2];
    p.w = vals[3];
}

namespace SimpleMath
{
    inline void from_json(const nlohmann::json& j, Vector4& p)
    {
        const auto vals = j.get<std::array<float, 4>>();
        p.x = vals[0];
        p.y = vals[1];
        p.z = vals[2];
        p.w = vals[3];
    }

    inline void from_json(const nlohmann::json& j, Color& p)
    {
        const auto vals = j.get<std::array<float, 3>>();
        p.x = vals[0];
        p.y = vals[1];
        p.z = vals[2];
    }


    inline void from_json(const nlohmann::json& j, Vector3& p)
    {
        const auto vals = j.get<std::array<float, 3>>();
        p.x = vals[0];
        p.y = vals[1];
        p.z = vals[2];
    }

    inline void from_json(const nlohmann::json& j, Vector2& p)
    {
        const auto vals = j.get<std::array<float, 2>>();
        p.x = vals[0];
        p.y = vals[1];
    }

    inline void from_json(const nlohmann::json& j, Quaternion& p)
    {
        const auto vals = j.get<std::array<float, 4>>();
        p.x = vals[0];
        p.y = vals[1];
        p.z = vals[2];
        p.w = vals[3];
    }
}

}

template<class T>
bool get_to_if_exists(const nlohmann::json& j, const std::string& key, T& dest)
{
    if (j.contains(key))
    {
        j.at(key).get_to(dest);
        return true;
    }

    return false;
}

namespace Snail
{ 
    template<class T>
    void from_json(const nlohmann::json& json, T& p);

    template<class T>
    void to_json(nlohmann::json& j, const T& p);
}