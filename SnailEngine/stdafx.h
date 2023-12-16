
// fichier Include pour les fichiers Include syst�me standard,
// ou les fichiers Include sp�cifiques aux projets qui sont utilis�s fr�quemment,
// et sont rarement modifi�s
//

#pragma once

#define WIN32_LEAN_AND_MEAN		// Exclure les en-t�tes Windows rarement utilis�s
#define NOMINMAX // Don't include the awful Windows min and max macros
// Fichiers d'en-t�te Windows�:
#include <windows.h>

// Fichiers d'en-t�te C RunTime
#include <cassert>
#include <cstdint>
#include <tchar.h>

// Fichiers d'en-t�te C++ RunTime
#include <exception>

#define _USE_MATH_DEFINES
#include "Audio.h"

#include <winsdkver.h>
#define _WIN32_WINNT 0x0A00
#include <sdkddkver.h>

#define _XM_NO_INTRINSICS_
#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include "Core/Math/SimpleMath.h"
using namespace DirectX::SimpleMath;

#include "Util/Logger.h"

// Only include these in debug mode
#ifdef _DEBUG

// Activate this line for more detailled textures for debugging.
// Disabled to reduce console clutter.
// #define _PRIVATE_DATA

#define _CRTDBG_MAP_ALLOC 1
#include <stdlib.h>
#include <crtdbg.h>

//#define _RENDERDOC_
//#include "renderdoc_app.h"


// ImGui is included
#define _IMGUI_

#endif

// ImGui is included
//#define _IMGUI_ // FOR DEBUG PURPOSES IN RELEASE

#ifdef _IMGUI_

// Ignore warnings in ImGui code
#pragma warning(push, 0)
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>
#pragma warning(pop)

#endif
