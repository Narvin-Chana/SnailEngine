//--------------------------------------------------------------------------------------
// File: DirectXHelpers.cpp
//
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// http://go.microsoft.com/fwlink/?LinkId=248929
//--------------------------------------------------------------------------------------

#include "pch.h"
#include "DirectXHelpers.h"
#include "Effects.h"
#include "PlatformHelpers.h"


using namespace DirectX;

_Use_decl_annotations_
HRESULT DirectX::CreateInputLayoutFromEffect(
    ID3D11Device*,
    IEffect*,
    const D3D11_INPUT_ELEMENT_DESC*,
    size_t,
    ID3D11InputLayout**) noexcept
{
    // CODE OVERRIDEN MANUALLY AS IT SERVES NO TANGIBLE PURPOSE FOR AUDIO
    return -1;
}
