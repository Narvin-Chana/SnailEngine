#pragma once

namespace Snail {

class DeviceInfo {
	bool valid;
	int width;
	int height;

	int memory;
	wchar_t cardName[100];

	DXGI_MODE_DESC mode;
public:
	enum class DeviceType {
		CURRENT_ADAPTER = 0
	};

	explicit DeviceInfo(DeviceType adapterNumber);
	DeviceInfo(DXGI_MODE_DESC modeDesc);

	bool IsValid() const noexcept { return valid; }
	int GetWidth() const noexcept { return width; }
	int GetHeight() const noexcept { return height; }
	int GetMemory() const noexcept { return memory; }
	const wchar_t* GetCardName() const noexcept { return cardName; }

	void GetDesc(DXGI_MODE_DESC& modeDesc) { modeDesc = mode; }
};

};