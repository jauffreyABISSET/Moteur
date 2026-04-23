#include "pch.h"
#include "InputsMethods.h"

XMFLOAT2 GetMousePosition()
{
    POINT p;
    GetCursorPos(&p);
    ScreenToClient(InputSystem::s_windowHandle, &p);
    return XMFLOAT2(static_cast<float>(p.x), static_cast<float>(p.y));
}

void SetMousePosition(XMFLOAT2 const& coordinates)
{
    POINT p{ static_cast<long>(coordinates.x), static_cast<long>(coordinates.y) };
    ClientToScreen(InputSystem::s_windowHandle, &p);
    SetCursorPos(p.x, p.y);
}

void ShowMouseCursor(bool showMouse)
{
    switch (showMouse) {
    case true: {
        while (InputSystem::s_cursorVisibilityCount < 0)
            InputSystem::s_cursorVisibilityCount = ShowCursor(TRUE);
        break;
    }
    case false: {
        while (InputSystem::s_cursorVisibilityCount >= 0)
            InputSystem::s_cursorVisibilityCount = ShowCursor(FALSE);
        break;
    }
    }
}

XMFLOAT2 mLastMousePos = { 0.f, 0.f };

XMFLOAT2 LockMouseCursor(bool locked)
{
	XMFLOAT2 mouse = GetMousePosition();

	GameManager* gm = GameManager::GetInstance();
	Window& window = gm->GetWindow();

	float centerX = window.GetClientWidth() / 2.0f;
	float centerY = window.GetClientHeight() / 2.0f;

	float dx = mouse.x - centerX;
	float dy = mouse.y - centerY;

	if (locked) {
        SetMousePosition(XMFLOAT2(centerX, centerY));
	}

	mLastMousePos = XMFLOAT2(centerX, centerY);

	return XMFLOAT2(dx, dy);
}
