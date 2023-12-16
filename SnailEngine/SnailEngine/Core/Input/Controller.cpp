#include "stdafx.h"
#include "Controller.h"

#include <algorithm>
#include <limits>

namespace Snail
{
void ControllerManager::DiscoverControllers()
{
	for (int i = 0; i < XUSER_MAX_COUNT; ++i)
	{
		if (auto input = GetXInputGamepad(i); input)
		{
			if (!Controllers[i])
			{
                //LOGF("Controller connected at slot {}", i);
				Controllers[i] = Controller{};
			}
			Controllers[i]->SetState(*input);
		}
		else
		{
            //LOGF("Controller disconnected at slot {}", i);
			Controllers[i] = {};
		}
	}
}

void ControllerManager::Update()
{
	for (int i = 0; i < XUSER_MAX_COUNT; ++i)
	{
		if (Controllers[i]) {
			if (auto input = GetXInputGamepad(i); input) {
				Controllers[i]->SetState(*input);
			}
			else {
				Controllers[i] = {};
			}
		}
	}
}

const Controller* ControllerManager::GetController(int controllerId)
{
	if (controllerId < 0 || controllerId > Controllers.size())
		return nullptr;

	std::ranges::partition(Controllers, [](const std::optional<Controller>& cont1) {
		return cont1.has_value();
	});

	const auto& val = Controllers[controllerId];
	if (!val)
		return nullptr;

	return &*val;
}

const Controller* ControllerManager::GetFirstActiveController()
{
	auto it = std::find_if(Controllers.begin(), Controllers.end(), [](const std::optional<Controller>& cont1) {
		return cont1.has_value();
	});

	if (it == Controllers.end())
		return nullptr;

	return &**it;
}

DirectX::XMFLOAT2 Controller::GetLeftJoystick() const
{
	return {
		(float)GamepadState.sThumbLX / (std::numeric_limits<SHORT>::max)(),
		(float)GamepadState.sThumbLY / (std::numeric_limits<SHORT>::max)()
	};
}

DirectX::XMFLOAT2 Controller::GetRightJoystick() const
{
	return {
		(float)GamepadState.sThumbRX / (std::numeric_limits<SHORT>::max)(),
		(float)GamepadState.sThumbRY / (std::numeric_limits<SHORT>::max)()
	};
}

float Controller::GetLeftTrigger() const
{
	return (float)GamepadState.bLeftTrigger / 255.0f;
}

float Controller::GetRightTrigger() const
{
	return (float)GamepadState.bRightTrigger / 255.0f;
}

bool Controller::IsPressed(Buttons button) const
{
	return IsDown(GamepadState, button) && !IsDown(PrevGamepadState, button);
}

bool Controller::IsDown(Buttons button) const
{
	return IsDown(GamepadState, button);
}

bool Controller::IsUp(Buttons button) const
{
	return !IsDown(GamepadState, button) && IsDown(PrevGamepadState, button);
}

bool Controller::IsNeutral() const
{
	XINPUT_GAMEPAD idle{};
	return memcmp(&GamepadState, &idle, sizeof(XINPUT_GAMEPAD));
}

std::optional<XINPUT_GAMEPAD> ControllerManager::GetXInputGamepad(int id)
{
	XINPUT_STATE GamepadState;
	ZeroMemory(&GamepadState, sizeof(XINPUT_STATE));
	if (XInputGetState(id, &GamepadState) == ERROR_SUCCESS)
		return GamepadState.Gamepad;

	return {};
}

void Controller::SetState(const XINPUT_GAMEPAD& state)
{
	PrevGamepadState = GamepadState;
	GamepadState = state;
}

bool Controller::IsDown(const XINPUT_GAMEPAD& state, Buttons button) const
{
	switch (button)
	{
	case Buttons::A: return state.wButtons & XINPUT_GAMEPAD_A;
	case Buttons::B: return state.wButtons & XINPUT_GAMEPAD_B;
	case Buttons::X: return state.wButtons & XINPUT_GAMEPAD_X;
	case Buttons::Y: return state.wButtons & XINPUT_GAMEPAD_Y;
	case Buttons::D_UP: return state.wButtons & XINPUT_GAMEPAD_DPAD_UP;
	case Buttons::D_DOWN: return state.wButtons & XINPUT_GAMEPAD_DPAD_DOWN;
	case Buttons::D_LEFT: return state.wButtons & XINPUT_GAMEPAD_DPAD_LEFT;
	case Buttons::D_RIGHT: return state.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT;
	case Buttons::RB: return state.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER;
	case Buttons::RT: return state.bRightTrigger > 0;
	case Buttons::LB: return state.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER;
	case Buttons::LT: return state.bLeftTrigger > 0;
	case Buttons::START: return state.wButtons & XINPUT_GAMEPAD_START;
	case Buttons::SELECT: return state.wButtons & XINPUT_GAMEPAD_BACK;
	}

	return false;
}

}
