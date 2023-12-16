#pragma once

#include "Rendering/D3D11Device.h"
#include "Engine.h"
#include "Clock.h"
#include "Util/SnailException.h"

namespace Snail
{
#define MAX_LOADSTRING 100

class WindowsEngine final : public Engine<WindowsEngine, D3D11Device>
{
    friend Singleton;

public:
    using Engine::GetModule;

    static void SetWindowsAppInstance(HINSTANCE hInstance);
    void ResetClock() override;
    HWND GetHwnd() const { return hMainWnd; }
    double GetElapsedFrameTime() const
    {
        return GetTimeIntervalsInSec(prevTime, GetTimeSpecific());
    }

    /**
     * \brief Exception handler for HRESULTS (codes received via WinAPI)
     */
    class HRException final : public SnailException
    {
    public:
        HRException(unsigned int line, const char* file, HRESULT hr) noexcept;
        const char* what() const noexcept override;
        const char* GetType() const noexcept override;
        HRESULT GetErrorCode() const noexcept;
        std::string GetErrorDescription() const noexcept;
        /**
         * \brief Uses WinAPI to translate an error code into a readable format.
         * \param hr HRESULT error code
         * \return string of the new error code
         */
        static std::string TranslateErrorCode(HRESULT hr) noexcept;

    private:
        HRESULT hr;
    };
private:
    ~WindowsEngine() override;

    bool InitAppInstance();
    ATOM MyRegisterClass(HINSTANCE hInstance);
    int Show();

    // Les fonctions sp�cifiques
    int InitSpecific() override;
    void UpdateSpecific() override;
    int64_t GetTimeSpecific() const override;
    double GetTimeIntervalsInSec(int64_t start, int64_t stop) const override;

    // Fonctions "Callback" -- Doivent �tre statiques
    static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
    static INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);

    HACCEL hAccelTable = nullptr; // handle Windows de la table des acc�l�rateurs
    static HINSTANCE hAppInstance; // handle Windows de l'instance actuelle de l'application
    HWND hMainWnd = nullptr; // handle Windows de la fenêtre principale
    TCHAR szWindowClass[MAX_LOADSTRING] = {}; // le nom de la classe de fenêtre principale

#ifdef _RENDERDOC_
    HINSTANCE RenderDocDLL{};
    RENDERDOC_API_1_1_2* rdoc_api = nullptr;
#endif

    Clock clock;

    D3D11Device* CreateDeviceSpecific(Device::DisplayMode mode) override;
    void ResizeSpecific() override;

    UINT resizeWidth = 0, resizeHeight = 0;
};

// Calls HRException with line and file to avoid having to type them in every time.
#define WND_EXCEPT(hr) WindowsEngine::HRException(__LINE__, __FILE__, hr)

} // namespace Snail
