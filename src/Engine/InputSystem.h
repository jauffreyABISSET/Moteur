#pragma once
#include "InputsEnums.h"
#include <DirectXMath.h>
#include <DirectXPackedVector.h>

using namespace DirectX;

class InputSystem final
{
public:
    enum States : int {
        NONE    = 0b00,
        PRESSED = 0b01,
        UP      = 0b10,
        DOWN    = 0b11
    };
    static void HandleInputs();
private:


	inline static States s_mouseStates[Mouse::AMOUNT];
	inline static States s_keyboardStates[Keyboard::AMOUNT];

    inline static bool s_cursorLocked = false;
    inline static bool s_cursorVisible = true;
    inline static HWND s_windowHandle = nullptr;
    inline static int s_cursorVisibilityCount = 0;

    friend bool GetButton(Mouse::Button button);
    friend bool GetButtonUp(Mouse::Button button);
    friend bool GetButtonDown(Mouse::Button button);

    friend XMFLOAT2 GetMousePosition();
    friend void SetMousePosition(XMFLOAT2 const& coordinates);

    friend bool IsMouseCursorLocked();
    friend bool IsMouseCursorVisible();

    friend XMFLOAT2 LockMouseCursor(bool isLocked);
    friend void ShowMouseCursor(bool isShowed);

    friend bool GetKey(Keyboard::Key key);
    friend bool GetKeyUp(Keyboard::Key key);
    friend bool GetKeyDown(Keyboard::Key key);
};

