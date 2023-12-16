#include "stdafx.h"
#include <algorithm>
#include <chrono>

#include "WindowsEngine.h"

#include "Core/Input/InputModule.h"

#include "WindowsResource/resource.h"

#ifdef _IMGUI_
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
#endif

namespace Snail
{
// Windows app instance handle
HINSTANCE WindowsEngine::hAppInstance;

void WindowsEngine::SetWindowsAppInstance(const HINSTANCE hInstance) { hAppInstance = hInstance; }

bool WindowsEngine::InitAppInstance()
{
    TCHAR szTitle[MAX_LOADSTRING]; // Le texte de la barre de titre

    // Initialise les chaénes globales
    LoadString(hAppInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadString(hAppInstance, IDC_PETITMOTEUR3D, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hAppInstance);

    hMainWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hAppInstance, NULL);

    if (!hMainWnd)
        throw WND_EXCEPT(GetLastError());

    hAccelTable = LoadAccelerators(hAppInstance, MAKEINTRESOURCE(IDC_PETITMOTEUR3D));

    if (!hAccelTable)
        throw WND_EXCEPT(GetLastError());

    return true;
}

ATOM WindowsEngine::MyRegisterClass(const HINSTANCE hInstance)
{
    WNDCLASSEX wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = &WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SNAIL_ENGINE));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCE(IDC_PETITMOTEUR3D);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    SetCursor(wcex.hCursor);

    return RegisterClassEx(&wcex);
}

int WindowsEngine::Show()
{
    ShowWindow(hMainWnd, SW_SHOWNORMAL);
    UpdateWindow(hMainWnd);

    return 0;
}

void WindowsEngine::UpdateSpecific()
{
    MSG msg;
    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
    {
        if (msg.message == WM_QUIT) { shouldExit = true; }

        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    if (resizeWidth != 0 && resizeHeight != 0)
    {
        std::lock_guard lock(DeviceMutex);
        Resize(resizeWidth, resizeHeight);
        resizeWidth = resizeHeight = 0;
    }
}

int64_t WindowsEngine::GetTimeSpecific() const { return clock.GetTimeCount(); }

double WindowsEngine::GetTimeIntervalsInSec(const int64_t start, const int64_t stop) const
{
    const double rawTime = clock.GetTimeBetweenCounts(start, stop);
    return std::min(rawTime, 0.5); // we limit the frame time to 0.5s (2FPS)
}

int WindowsEngine::InitSpecific()
{
    // Initialisations de l'application;
    InitAppInstance();

    const auto& inputModule = InputModule::GetInstance();
    inputModule.Mouse.SetWindow(hMainWnd);

    Show();

    RAWINPUTDEVICE dev[1];
    dev[0].usUsagePage = 0x01; // HID_USAGE_PAGE_GENERIC
    dev[0].usUsage = 0x02; // HID_USAGE_GENERIC_MOUSE
    dev[0].dwFlags = 0; // adds mouse and also ignores legacy mouse messages
    dev[0].hwndTarget = hMainWnd;

    if (!RegisterRawInputDevices(dev, 1, sizeof(RAWINPUTDEVICE))) { throw WND_EXCEPT(GetLastError()); };

    return 0;
}

LRESULT CALLBACK WindowsEngine::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    HDC hdc;

#ifdef _IMGUI_
    ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam);

    if (ImGui::GetCurrentContext() == nullptr)
    // We return early, because if the context does not exist yet and we try to get the IO, it is invalid behavior.
    {
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

#endif

    static WindowsEngine& engine = GetInstance();
    static InputModule& inputModule = InputModule::GetInstance();

    inputModule.Mouse.ProcessMessage(message, wParam, lParam);
    inputModule.Keyboard.ProcessMessage(message, wParam, lParam);

    switch (message)
    {
    case WM_SYSKEYDOWN:
        {
            if ((wParam == VK_RETURN && (lParam & (1 << 29)))) {
                // Alt+Enter was pressed (lParam bit 29 is set)
                engine.ToggleFullscreen();
                return 0; // Prevent default processing
            }
        }
        break;
    case WM_SIZE:
        if (wParam == SIZE_MINIMIZED)
            return 0;
        engine.resizeWidth = static_cast<UINT>(LOWORD(lParam)); // Queue resize
        engine.resizeHeight = static_cast<UINT>(HIWORD(lParam));
        return 0;
    case WM_COMMAND:
        return DefWindowProc(hWnd, message, wParam, lParam);
    case WM_PAINT:
        hdc = BeginPaint(hWnd, &ps);
        // évitez d'ajouter du code ici...
        EndPaint(hWnd, &ps);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;
}

D3D11Device* WindowsEngine::CreateDeviceSpecific(const Device::DisplayMode mode) { return new D3D11Device(mode, hMainWnd); }

void WindowsEngine::ResizeSpecific()
{
}

void WindowsEngine::ResetClock() { prevTime = nextTime = GetTimeSpecific(); }

WindowsEngine::~WindowsEngine() = default;

// HRException Definitions
WindowsEngine::HRException::HRException(const unsigned int line, const char* file, const HRESULT hr) noexcept
    : SnailException(line, file)
  , hr(hr) {}

const char* WindowsEngine::HRException::what() const noexcept
{
    std::ostringstream oss;
    oss << GetType() << std::endl << "[Error Code] 0x" << std::hex << std::uppercase << GetErrorCode() << std::dec << " (" << static_cast<unsigned long>(GetErrorCode()) << ")" << std::endl << "[Description] " << GetErrorDescription() << std::endl << GetOriginString();
    whatBuffer = oss.str();
    return whatBuffer.c_str();
}

const char* WindowsEngine::HRException::GetType() const noexcept { return "Windows Exception"; }

HRESULT WindowsEngine::HRException::GetErrorCode() const noexcept { return hr; }

std::string WindowsEngine::HRException::GetErrorDescription() const noexcept { return TranslateErrorCode(hr); }

std::string WindowsEngine::HRException::TranslateErrorCode(const HRESULT hr) noexcept
{
    char* pMsgBuf = nullptr;
    // windows will allocate memory for err string and make our pointer point to it
    // 0 string length returned indicates a failure
    if (const DWORD nMsgLen = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), reinterpret_cast<LPWSTR>(&pMsgBuf), 0, nullptr); nMsgLen == 0) { return "Unidentified error code"; }
    // copy error string from windows-allocated buffer to std::string
    std::string errorString = pMsgBuf;
    // free windows buffer
    LocalFree(pMsgBuf);
    return errorString;
}

} // namespace Snail
