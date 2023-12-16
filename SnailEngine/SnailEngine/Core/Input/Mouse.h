#pragma once
#pragma once

#include "Util/Singleton.h"
#include <wtypes.h>
#include <chrono>
#include <mutex>

namespace Snail
{
	class Mouse final : public Singleton<Mouse>
	{
	public:
		/**
		 * \brief Determines if the mouse should be used to move the camera or if it is used to click on UI.
		 * Relative = Think of how the mouse works in an FPS
		 * Absolute = Think of how the mouse works on your desktop
		 */
		enum Mode
		{
			MODE_ABSOLUTE,
			MODE_RELATIVE
		};

		enum StateChange {
			NONE = 0,
			PRESSED,
			RELEASED,
			DOWN,
		};

		/**
		 * \brief Mouse button states used to get the specific pressed or released value from the mouse state tracker.
		 */
		struct MouseButtonState
		{
			bool leftButton;
			bool middleButton;
			bool rightButton;
			bool xButton1; // Generally called the back button
			bool xButton2; // Generally called the forwards button
		};

		template <typename StateType>
		struct MouseStateTracker
		{
			int x{};
			int y{};
			int scrollWheelDelta{};
			int scrollWheelValue{};
			Mode mode = Mode::MODE_ABSOLUTE;

			StateType state;

			// Update the states with new input data
			void Update(const StateType& newState)
			{
				state = newState;
			}

			// Update a specific member with a new value
			template <typename MemberType>
			void Update(MemberType StateType::* member, bool newValue)
			{
				StateType newState = state;
				newState.*member = newValue;
				state = newState;
			}

			const StateType& GetCurrentState() const
			{
				return state;
			}
		};

		StateChange GetButtonStateFlags(bool MouseButtonState::* member) const noexcept;

		bool IsButtonPressed(bool MouseButtonState::* member) const noexcept;
		bool IsStatePressed(StateChange change) const noexcept;

		bool IsButtonReleased(bool MouseButtonState::* member) const noexcept;
		bool IsStateReleased(StateChange change) const noexcept;

		bool IsButtonDown(bool MouseButtonState::* member) const noexcept;
		bool IsStateDown(StateChange change) const noexcept;

        Vector2 GetMousePosition() const;

        MouseStateTracker<MouseButtonState> GetMouseState() const { return frameTracker; }

		bool IsVisible() const noexcept;
		void SetVisible(bool visible);
        bool IsLocked() const noexcept;
		void SetLock(bool locked);
		void SetMode(Mode mode);

		void Update() noexcept;

		/**
		 * \brief Used to process WndProc's mouse related events.
		 */
		void ProcessMessage(UINT message, WPARAM wParam, LPARAM lParam);

		/**
		 * \brief Sets the window in which the mouse is.
		 * \param wnd HWND window
		 */
		void SetWindow(HWND wnd);

    protected:
		/**
		 * \brief In relative mode keeps the mouse inside window to avoid clicking out and alt-tabbing.
		 */
		void ClipToWindow() const;

	private:
		void SetCursorMiddle() const;

		MouseStateTracker<MouseButtonState> ongoingTracker;

		MouseStateTracker<MouseButtonState> frameTracker;
		MouseStateTracker<MouseButtonState> prevFrameTracker;

		bool isVisible = true;
		bool isInFocus = true;
		bool isLockedCenter = false;

		int relativeX = 0, relativeY = 0;

		HWND window = nullptr;
	};

}