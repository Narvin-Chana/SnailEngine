#pragma once
#include <optional>
#include <array>

#include "Util/Singleton.h"

#include "Xinput.h"

namespace Snail
{
class ControllerManager;

class Controller
{
public:
	enum class Buttons
	{
		A, B, X, Y,
		D_UP, D_DOWN, D_LEFT, D_RIGHT,
		RB, RT, LB, LT,
		START, SELECT
	};
private:
	friend ControllerManager;

	XINPUT_GAMEPAD PrevGamepadState;
	XINPUT_GAMEPAD GamepadState;
	void SetState(const XINPUT_GAMEPAD& state);

	bool IsDown(const XINPUT_GAMEPAD& state, Buttons button) const;

	Controller() = default;
public:
	Controller(const Controller&) = delete;
	Controller& operator=(const Controller&) = delete;

	Controller(Controller&&) = default;
	Controller& operator=(Controller&&) = default;

	DirectX::XMFLOAT2 GetLeftJoystick() const;
	DirectX::XMFLOAT2 GetRightJoystick() const;

	float GetLeftTrigger() const;
	float GetRightTrigger() const;

	bool IsPressed(Buttons button) const;
	bool IsDown(Buttons button) const;
	bool IsUp(Buttons button) const;
	bool IsNeutral() const;
};
	
class ControllerManager : public Singleton<ControllerManager>
{
	std::array<std::optional<Controller>, XUSER_MAX_COUNT> Controllers{ };
	std::optional<XINPUT_GAMEPAD> GetXInputGamepad(int id);
public:
	void DiscoverControllers();
	void Update();

	const Controller* GetController(int controllerId);
	const Controller* GetFirstActiveController();
};

}
