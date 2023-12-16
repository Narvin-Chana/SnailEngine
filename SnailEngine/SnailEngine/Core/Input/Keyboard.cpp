#include "stdafx.h"
#include "Keyboard.h"

namespace Snail
{

	inline void KeyDown(const int key, Keyboard::KeyboardState& state) noexcept
	{
		state.keys[key] = true;
	}

	inline void KeyUp(const int key, Keyboard::KeyboardState& state) noexcept
	{
		state.keys[key] = false;
	}

    void Keyboard::Update()
	{
        keyboardStateTracker.previousState = keyboardStateTracker.currentState;
	}

	void Keyboard::ProcessMessage(const UINT message, const WPARAM wParam, const LPARAM lParam)
	{
		bool down = false;

		switch (message)
		{
		case WM_ACTIVATE:
		case WM_ACTIVATEAPP:
			keyboardStateTracker.Reset();
			return;
		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
			down = true;
			break;
		case WM_KEYUP:
		case WM_SYSKEYUP:
			break;
		default:
			// Unsupported behavior
			return;
		}

		int vk = LOWORD(wParam);

		switch (vk)
		{
		case VK_SHIFT:
		case VK_CONTROL:
		case VK_MENU:
		{
			if (vk == VK_SHIFT && !down)
			{
				// Workaround to ensure left vs. right shift get cleared when both were pressed at same time
				KeyboardState state = keyboardStateTracker.currentState;
				KeyUp(VK_LSHIFT, state);
				KeyUp(VK_RSHIFT, state);
				keyboardStateTracker.Update(state);
			}

			const bool isExtendedKey = (HIWORD(lParam) & KF_EXTENDED) == KF_EXTENDED;
			const int scanCode = LOBYTE(HIWORD(lParam)) | (isExtendedKey ? 0xe000 : 0);
			vk = LOWORD(MapVirtualKeyW(static_cast<UINT>(scanCode), MAPVK_VSC_TO_VK_EX));
		}
		break;
		default:
			// No control keys pressed
			break;
		}

		if (down)
		{
			KeyboardState state = keyboardStateTracker.currentState;
			KeyDown(vk, state);
			keyboardStateTracker.Update(state);
		}
		else
		{
			KeyboardState state = keyboardStateTracker.currentState;
			KeyUp(vk, state);
			keyboardStateTracker.Update(state);
		}
	}

	const Keyboard::KeyboardStateTracker<Keyboard::KeyboardState>& Keyboard::GetState() const
	{
		return keyboardStateTracker;
	}

}