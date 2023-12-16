#include "stdafx.h"
#include "Mouse.h"
#include "Util/DebugUtil.h"

#include <tuple>
#include <cassert>
#include <chrono>

namespace Snail
{
Mouse::StateChange Mouse::GetButtonStateFlags(bool MouseButtonState::* member) const noexcept
{
    const bool pressedNow = frameTracker.state.*member;
    const bool pressedBefore = prevFrameTracker.state.*member;

    int val = StateChange::NONE;

    if (pressedNow && !pressedBefore)
    {
        val |= 1 << StateChange::PRESSED;
    }

    if (!pressedNow && pressedBefore)
    {
        val |= 1 << StateChange::RELEASED;
    }

    if (pressedNow)
    {
        val |= 1 << StateChange::DOWN;
    }

    return static_cast<Mouse::StateChange>(val);
}

bool Mouse::IsButtonPressed(bool MouseButtonState::* member) const noexcept
{
    auto flags = GetButtonStateFlags(member);
    return IsStatePressed(flags);
}

bool Mouse::IsButtonReleased(bool MouseButtonState::* member) const noexcept
{
    auto flags = GetButtonStateFlags(member);
    return IsStateReleased(flags);
}

bool Mouse::IsStateReleased(StateChange change) const noexcept
{
    return change & (1 << StateChange::RELEASED);
}

bool Mouse::IsStatePressed(StateChange change) const noexcept
{
    return change & (1 << StateChange::PRESSED);
}

bool Mouse::IsButtonDown(bool MouseButtonState::* member) const noexcept
{
    auto flags = GetButtonStateFlags(member);
    return IsStateDown(flags);
}

bool Mouse::IsStateDown(StateChange change) const noexcept
{
    return change & (1 << StateChange::DOWN);
}

Vector2 Mouse::GetMousePosition() const
{
    POINT pos;
    if (!GetCursorPos(&pos))
        return {};

    if (!ScreenToClient(window, &pos))
        return {};

    return Vector2{ static_cast<float>(pos.x), static_cast<float>(pos.y) };
}

bool Mouse::IsVisible() const noexcept
{
    return isVisible;
}

void Mouse::SetVisible(const bool visible)
{
    if (visible != isVisible)
    {
        ShowCursor(visible);
        isVisible = visible;
    }
}
bool Mouse::IsLocked() const noexcept { return isLockedCenter; }

void Mouse::SetLock(const bool locked)
{
    isLockedCenter = locked;
    if (isLockedCenter)
        SetCursorMiddle();
}

void Mouse::SetMode(Mode mode)
{
    ongoingTracker.mode = mode;
    frameTracker.x = frameTracker.y = 0;
}

void Mouse::Update() noexcept
{
    prevFrameTracker = frameTracker;
    frameTracker = ongoingTracker;
    ongoingTracker.x = ongoingTracker.y = ongoingTracker.scrollWheelDelta = ongoingTracker.scrollWheelValue = 0;

    if (isLockedCenter)
        SetCursorMiddle();
}

void Mouse::ProcessMessage(const UINT message, const WPARAM wParam, const LPARAM lParam)
{
#ifdef _IMGUI_
    static auto io = ImGui::GetIO();
#endif

    // We have to get the IO object from ImGUI to see if it is capturing mouse/keyboard
    switch (message)
    {
    case WM_ACTIVATE:
    case WM_ACTIVATEAPP:
        isInFocus = wParam != WA_INACTIVE;
        if (isInFocus)
        {
            if (ongoingTracker.mode == MODE_RELATIVE)
            {
                SetLock(true);
                SetVisible(false);
            }
        }
        else
        {
            SetLock(false);
            SetVisible(true);
        }
        ongoingTracker = {};
        return;
    case WM_INPUT:
#ifdef _IMGUI_
        // Let ImGui take priority if it wants it
        if (io.WantCaptureMouse) { break; }
#endif

        if (isInFocus && ongoingTracker.mode == MODE_RELATIVE)
        {
            RAWINPUT raw;
            UINT rawSize = sizeof(raw);

            // Load data into raw
            if (const UINT resultData = GetRawInputData(
                reinterpret_cast<HRAWINPUT>(lParam),
                RID_INPUT,
                &raw,
                &rawSize,
                sizeof(RAWINPUTHEADER)
            ); resultData == static_cast<UINT>(-1))
            {
                // Bad data return
                return;
            }
            if (raw.header.dwType == RIM_TYPEMOUSE)
            {
                if (!(raw.data.mouse.usFlags & MOUSE_MOVE_ABSOLUTE))
                {
                    ongoingTracker.x += raw.data.mouse.lLastX;
                    ongoingTracker.y += raw.data.mouse.lLastY;
                    // TODO: Reset relativeRead event
                }
                else if (raw.data.mouse.usFlags & MOUSE_VIRTUAL_DESKTOP)
                {
                    // This is used to make Remote Desktop sessons work
                    const int width = GetSystemMetrics(SM_CXVIRTUALSCREEN);
                    const int height = GetSystemMetrics(SM_CYVIRTUALSCREEN);

                    const int x = static_cast<int>(static_cast<float>(raw.data.mouse.lLastX) / 65535.0f * static_cast<
                        float>(width));
                    const int y = static_cast<int>(static_cast<float>(raw.data.mouse.lLastY) / 65535.0f * static_cast<
                        float>(height));

                    if (relativeX == INT32_MAX)
                    {
                        ongoingTracker.x = ongoingTracker.y = 0;
                    }
                    else
                    {
                        ongoingTracker.x += x - relativeX;
                        ongoingTracker.y += y - relativeY;
                    }

                    relativeX = x;
                    relativeY = y;

                    // TODO: Reset relativeRead event
                }
            }
        }
        return;
    case WM_MOUSEMOVE: DefWindowProc(window, message, wParam, lParam);
        break;
    case WM_LBUTTONDOWN:
        ongoingTracker.Update(&MouseButtonState::leftButton, true);
        break;
    case WM_LBUTTONUP:
        ongoingTracker.Update(&MouseButtonState::leftButton, false);
        break;
    case WM_RBUTTONDOWN:
        ongoingTracker.Update(&MouseButtonState::rightButton, true);
        break;
    case WM_RBUTTONUP:
        ongoingTracker.Update(&MouseButtonState::rightButton, false);
        break;
    case WM_MBUTTONDOWN:
        ongoingTracker.Update(&MouseButtonState::middleButton, true);
        break;
    case WM_MBUTTONUP:
        ongoingTracker.Update(&MouseButtonState::middleButton, false);
        break;
    case WM_MOUSEWHEEL:
        {
            const short delta = GET_WHEEL_DELTA_WPARAM(wParam);
            ongoingTracker.scrollWheelDelta = delta;
            ongoingTracker.scrollWheelValue += delta;
            return;
        }
    case WM_XBUTTONDOWN:
        switch (GET_XBUTTON_WPARAM(wParam))
        {
        case XBUTTON1:
            ongoingTracker.Update(&MouseButtonState::xButton1, true);
            break;
        case XBUTTON2:
            ongoingTracker.Update(&MouseButtonState::xButton2, true);
            break;
        default:
            // Not a valid mouse message
            break;
        }
        break;
    case WM_XBUTTONUP:
        switch (GET_XBUTTON_WPARAM(wParam))
        {
        case XBUTTON1:
            ongoingTracker.Update(&MouseButtonState::xButton1, false);
            break;
        case XBUTTON2:
            ongoingTracker.Update(&MouseButtonState::xButton2, false);
            break;
        default:
            // Not a valid mouse message
            break;
        }
        break;
    case WM_MOUSEHOVER:
        break;
    default:
        // Not a valid mouse message
        return;
    }

    if (ongoingTracker.mode == MODE_ABSOLUTE)
    {
        ongoingTracker.x = static_cast<short>(LOWORD(lParam));
        ongoingTracker.y = static_cast<short>(HIWORD(lParam));
    }
}

void Mouse::SetWindow(const HWND wnd)
{
    window = wnd;
}

void Mouse::ClipToWindow() const
{
    assert(window != nullptr);

    RECT rect;
    GetClientRect(window, &rect);

    POINT ul;
    ul.x = rect.left;
    ul.y = rect.top;

    POINT lr;
    lr.x = rect.right;
    lr.y = rect.bottom;

    std::ignore = MapWindowPoints(window, nullptr, &ul, 1);
    std::ignore = MapWindowPoints(window, nullptr, &lr, 1);

    rect.left = ul.x;
    rect.top = ul.y;

    rect.right = lr.x;
    rect.bottom = lr.y;

    ClipCursor(&rect);
}

void Mouse::SetCursorMiddle() const
{
    RECT rect;
    GetWindowRect(window, &rect);

    RECT crect;
    GetClientRect(window, &crect);
    SetCursorPos(rect.left + (crect.right - crect.left) / 2, rect.top + (crect.bottom - crect.top) / 2);
}

}
