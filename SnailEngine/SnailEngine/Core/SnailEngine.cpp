#include "stdafx.h"
#include "WindowsEngine.h"
#include "SnailEngine.h"
#include "Util/Util.h"

using namespace Snail;



int APIENTRY _tWinMain(
	const HINSTANCE hInstance,
	const HINSTANCE hPrevInstance,
	const LPTSTR    lpCmdLine,
	const int       nShowCmd)
{
	// Pour ne pas avoir d'avertissement
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	UNREFERENCED_PARAMETER(nShowCmd);

#ifdef _DEBUG
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG);
#endif

	try
	{
		// Création de l'objet Moteur
		WindowsEngine& windowsEngine = WindowsEngine::GetInstance();

		// Spécifiques à une application Windows
		windowsEngine.SetWindowsAppInstance(hInstance);

		// Initialisation du moteur
		windowsEngine.Init();

		// Boucle d'application
		windowsEngine.Run();

		return 1;
	}
    catch (DXError<HRESULT, int>& err)
    {
        wchar_t szErrMsg[MAX_LOADSTRING];// Un message d'erreur selon le code
        if (err.overrideError.has_value()) {
            ::LoadString(hInstance, err.overrideError.value(), szErrMsg, MAX_LOADSTRING);
        }
        else {
            wcscpy_s(szErrMsg, L"UNKNOWN ERROR");
        }

        wchar_t finalMessage[1024];// Un message d'erreur selon le code
        wsprintf(finalMessage, L"%s: %s", szErrMsg, err.errorMessage.c_str());
        ::MessageBox(nullptr, finalMessage, L"Error", MB_ICONWARNING);
        return 98;
    }
    catch (const SnailException& e)
    {
        constexpr int bufferSize = 128;
        wchar_t message[bufferSize];

        size_t numCharacterConverted;
        mbstowcs_s(&numCharacterConverted, message, e.what(), bufferSize - 1);

        size_t numCharacterConvertedType;
        wchar_t type[bufferSize];
        mbstowcs_s(&numCharacterConvertedType, type, e.GetType(), bufferSize - 1);

        ::MessageBox(nullptr, message, type, MB_ICONERROR);

        return 99;
    }
    catch (const nlohmann::detail::exception& e)
    {
        constexpr int bufferSize = 128;
        wchar_t message[bufferSize];

        size_t numCharacterConverted;
        mbstowcs_s(&numCharacterConverted, message, e.what(), bufferSize - 1);

        size_t numCharacterConvertedType;
        wchar_t type[bufferSize];
        mbstowcs_s(&numCharacterConvertedType, type, "JSON Exception", bufferSize - 1);

        ::MessageBox(nullptr, message, type, MB_ICONERROR);

        return 99;
    }
    catch (const std::exception& e)
    {
        constexpr int bufferSize = 128;
        wchar_t message[bufferSize];

        size_t numCharacterConverted;
        mbstowcs_s(&numCharacterConverted, message, e.what(), bufferSize - 1);
        ::MessageBox(nullptr, message, L"Standard Exception", MB_ICONERROR);

        return 99;
    }
	catch (int errorCode)
	{
		wchar_t szErrMsg[MAX_LOADSTRING];// Un message d'erreur selon le code
		::LoadString(hInstance, errorCode, szErrMsg, MAX_LOADSTRING);
		::MessageBox(nullptr, szErrMsg, L"Error", MB_ICONWARNING);
		return 99; // POURQUOI 99???
	}
}
