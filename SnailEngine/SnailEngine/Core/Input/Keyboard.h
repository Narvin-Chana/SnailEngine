#pragma once

#include "Util/Singleton.h"
#include "Util/DebugUtil.h"

namespace Snail
{

	class Keyboard final : public Singleton<Keyboard>
	{
	public:
		enum Keys : unsigned char
		{
			NONE = 0,

			BACK = 0x8,
			TAB = 0X9,

			ENTER = 0XD,

			PAUSE = 0X13,
			CAPSLOCK = 0X14,
			KANA = 0X15,
			IMEON = 0X16,

			KANJI = 0X19,

			IMEOFF = 0X1A,
			ESCAPE = 0X1B,
			IMECONVERT = 0X1C,
			IMENOCONVERT = 0X1D,

			SPACE = 0X20,
			PAGEUP = 0X21,
			PAGEDOWN = 0X22,
			END = 0X23,
			HOME = 0X24,
			LEFT = 0X25,
			UP = 0X26,
			RIGHT = 0X27,
			DOWN = 0X28,
			SELECT = 0X29,
			PRINT = 0X2A,
			EXECUTE = 0X2B,
			PRINTSCREEN = 0X2C,
			INSERT = 0X2D,
			DDELETE = 0X2E,
			HELP = 0X2F,
			D0 = 0X30,
			D1 = 0X31,
			D2 = 0X32,
			D3 = 0X33,
			D4 = 0X34,
			D5 = 0X35,
			D6 = 0X36,
			D7 = 0X37,
			D8 = 0X38,
			D9 = 0X39,

			A = 0X41,
			B = 0X42,
			C = 0X43,
			D = 0X44,
			E = 0X45,
			F = 0X46,
			G = 0X47,
			H = 0X48,
			I = 0X49,
			J = 0X4A,
			K = 0X4B,
			L = 0X4C,
			M = 0X4D,
			N = 0X4E,
			O = 0X4F,
			P = 0X50,
			Q = 0X51,
			R = 0X52,
			S = 0X53,
			T = 0X54,
			U = 0X55,
			V = 0X56,
			W = 0X57,
			X = 0X58,
			Y = 0X59,
			Z = 0X5A,
			LEFTWINDOWS = 0X5B,
			RIGHTWINDOWS = 0X5C,
			APPS = 0X5D,

			SLEEP = 0X5F,
			NUMPAD0 = 0X60,
			NUMPAD1 = 0X61,
			NUMPAD2 = 0X62,
			NUMPAD3 = 0X63,
			NUMPAD4 = 0X64,
			NUMPAD5 = 0X65,
			NUMPAD6 = 0X66,
			NUMPAD7 = 0X67,
			NUMPAD8 = 0X68,
			NUMPAD9 = 0X69,
			MULTIPLY = 0X6A,
			ADD = 0X6B,
			SEPARATOR = 0X6C,
			SUBTRACT = 0X6D,

			DECIMAL = 0X6E,
			DIVIDE = 0X6F,
			F1 = 0X70,
			F2 = 0X71,
			F3 = 0X72,
			F4 = 0X73,
			F5 = 0X74,
			F6 = 0X75,
			F7 = 0X76,
			F8 = 0X77,
			F9 = 0X78,
			F10 = 0X79,
			F11 = 0X7A,
			F12 = 0X7B,
			F13 = 0X7C,
			F14 = 0X7D,
			F15 = 0X7E,
			F16 = 0X7F,
			F17 = 0X80,
			F18 = 0X81,
			F19 = 0X82,
			F20 = 0X83,
			F21 = 0X84,
			F22 = 0X85,
			F23 = 0X86,
			F24 = 0X87,

			NUMLOCK = 0X90,
			SCROLL = 0X91,

			LEFTSHIFT = 0XA0,
			RIGHTSHIFT = 0XA1,
			LEFTCONTROL = 0XA2,
			RIGHTCONTROL = 0XA3,
			LEFTALT = 0XA4,
			RIGHTALT = 0XA5,
			BROWSERBACK = 0XA6,
			BROWSERFORWARD = 0XA7,
			BROWSERREFRESH = 0XA8,
			BROWSERSTOP = 0XA9,
			BROWSERSEARCH = 0XAA,
			BROWSERFAVORITES = 0XAB,
			BROWSERHOME = 0XAC,
			VOLUMEMUTE = 0XAD,
			VOLUMEDOWN = 0XAE,
			VOLUMEUP = 0XAF,
			MEDIANEXTTRACK = 0XB0,
			MEDIAPREVIOUSTRACK = 0XB1,
			MEDIASTOP = 0XB2,
			MEDIAPLAYPAUSE = 0XB3,
			LAUNCHMAIL = 0XB4,
			SELECTMEDIA = 0XB5,
			LAUNCHAPPLICATION1 = 0XB6,
			LAUNCHAPPLICATION2 = 0XB7,

			OEMSEMICOLON = 0XBA,
			OEMPLUS = 0XBB,
			OEMCOMMA = 0XBC,
			OEMMINUS = 0XBD,
			OEMPERIOD = 0XBE,
			OEMQUESTION = 0XBF,
			OEMTILDE = 0XC0,

			OEMOPENBRACKETS = 0XDB,
			OEMPIPE = 0XDC,
			OEMCLOSEBRACKETS = 0XDD,
			OEMQUOTES = 0XDE,
			OEM8 = 0XDF,

			OEMBACKSLASH = 0XE2,

			PROCESSKEY = 0XE5,

			OEMCOPY = 0XF2,
			OEMAUTO = 0XF3,
			OEMENLW = 0XF4,

			ATTN = 0XF6,
			CRSEL = 0XF7,
			EXSEL = 0XF8,
			ERASEEOF = 0XF9,
			PLAY = 0XFA,
			ZOOM = 0XFB,

			PA1 = 0XFD,
			OEMCLEAR = 0XFE,
		};

		/**
		 * \brief Keyboard state used to represent the current state of each key on the keyboard.
		 */
		struct KeyboardState
		{
			bool keys[256];
		};

		/**
		 * \brief Tracks the state of the Keyboard. Contains keys with released/pressed states.
		 */
		template <typename StateType>
		struct KeyboardStateTracker
		{
			enum KeyState { 
				UP = 0, 
				DOWN, 
				RELEASED, 
				PRESSED 
			};

			StateType currentState;
			StateType previousState;

			// Constructor to initialize the states
			KeyboardStateTracker()
			{
				memset(currentState.keys, 0, sizeof(currentState.keys));
				memset(previousState.keys, 0, sizeof(previousState.keys));
			}

			// Update the states with new input data
			void Update(const StateType& newState)
			{
				previousState = currentState; // Store the previous state
				currentState = newState; // Update the current state
			}

			void Reset()
			{
				memset(currentState.keys, 0, sizeof(currentState.keys));
				memset(previousState.keys, 0, sizeof(previousState.keys));
			}

			// Check the state of a specific key
			KeyState GetKeyState(int key) const noexcept
			{
				const bool isPressedNow = currentState.keys[key];
				const bool isPressedBefore = previousState.keys[key];

				int state = KeyState::UP;
				
				if (isPressedNow && !isPressedBefore)
				{
					state |= 1 << KeyState::PRESSED;
				}
				if (!isPressedNow && isPressedBefore)
				{
					state |= 1 << KeyState::RELEASED;
				}
				if (isPressedNow)
				{
					state |= 1 << KeyState::DOWN;
				}

				return static_cast<KeyState>(state);
			}

			// Check if a key was pressed since the last update
			bool IsKeyPressed(const int key) const noexcept
			{
				return GetKeyState(key) & 1 << KeyState::PRESSED;
			}

			// Check if a key was released since the last update
			bool IsKeyReleased(const int key) const noexcept
			{
				return GetKeyState(key) & 1 << KeyState::RELEASED;
			}

			// Check if a key was released since the last update
			bool IsKeyDown(const int key) const noexcept
			{
				return GetKeyState(key) & 1 << KeyState::DOWN;
			}
		};

        void Update();

		void ProcessMessage(UINT message, WPARAM wParam, LPARAM lParam);

		const KeyboardStateTracker<KeyboardState>& GetState() const;

	private:
		KeyboardStateTracker<KeyboardState> keyboardStateTracker;

	};

}