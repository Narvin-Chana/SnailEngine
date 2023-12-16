#include "stdafx.h"
#include "DeviceInfo.h"
#include "Util/Util.h"

#include <vector>

namespace Snail {

DeviceInfo::DeviceInfo(DeviceType adapterNumber)
{
	IDXGIFactory* factory = nullptr;
	IDXGIAdapter* adapter = nullptr;
	IDXGIOutput* output = nullptr;

	valid = false;

	CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory);
	if (FAILED(factory->EnumAdapters(static_cast<int>(adapterNumber), &adapter))) return;

	// Monitor options
	if (FAILED(adapter->EnumOutputs(0, &output))) return;

	// Obtenir la description de l��tat courant
	DXGI_OUTPUT_DESC outDesc;
	output->GetDesc(&outDesc);
	width = outDesc.DesktopCoordinates.right - outDesc.DesktopCoordinates.left;
	height = outDesc.DesktopCoordinates.bottom - outDesc.DesktopCoordinates.top;

	valid = true;

	DXGI_ADAPTER_DESC Desc;
	adapter->GetDesc(&Desc);

	// dedicated memory in MB
	memory = static_cast<int>(Desc.DedicatedVideoMemory / 1024 / 1024);
	
	// GPU name
	wcscpy_s(cardName, 100, Desc.Description);

	DX_RELEASE(output);
	DX_RELEASE(adapter);
	DX_RELEASE(factory);
}

DeviceInfo::DeviceInfo(DXGI_MODE_DESC modeDesc)
{
	// �num�ration des adaptateurs
	IDXGIFactory* factory = nullptr;
	CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory);
	IDXGIAdapter* adapter;
	std::vector<IDXGIAdapter*> adapters;
	for (UINT i = 0; factory->EnumAdapters(i, &adapter) != DXGI_ERROR_NOT_FOUND; ++i)
	{
		adapters.push_back(adapter);
	}
	// On travaille presque toujours avec vAdapters[0]
	// � moins d�avoir plusieurs cartes non-hybrides
	*this = DeviceInfo(DeviceType::CURRENT_ADAPTER);
	// Obtenir la sortie 0 - le moniteur principal
	IDXGIOutput* output = nullptr;
	adapters[0]->EnumOutputs(0, &output);
	// Obtenir le mode le plus int�ressant
	output->FindClosestMatchingMode(&modeDesc, &modeDesc, nullptr);

	mode = modeDesc;

	// Faire le m�nage pour �viter les � memory leaks �
	DX_RELEASE(output);
	for (int i = 0; i < adapters.size(); ++i)
	{
		DX_RELEASE(adapters[i]);
	}
	DX_RELEASE(factory);
}

};

