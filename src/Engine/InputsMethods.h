#pragma once
#include "InputsEnums.h"
#include "Engine/InputSystem.h"

// Mouse
[[nodiscard]] inline bool GetButton(Mouse::Button const button) { return InputSystem::s_mouseStates[button] == InputSystem::States::PRESSED; }
[[nodiscard]] inline bool GetButtonUp(Mouse::Button const button) { return InputSystem::s_mouseStates[button] == InputSystem::States::UP; }
[[nodiscard]] inline bool GetButtonDown(Mouse::Button const button) { return InputSystem::s_mouseStates[button] == InputSystem::States::DOWN; }

[[nodiscard]] XMFLOAT2 GetMousePosition();
void SetMousePosition(XMFLOAT2 const& coordinates);

XMFLOAT2 LockMouseCursor(bool isLocked);
void ShowMouseCursor(bool isShowed);

[[nodiscard]] inline bool IsMouseCursorLocked() { return InputSystem::s_cursorLocked; }
[[nodiscard]] inline bool IsMouseCursorVisible() { return InputSystem::s_cursorVisible; }

// Keyboard
[[nodiscard]] inline bool GetKey(Keyboard::Key const key) { return InputSystem::s_keyboardStates[key] & InputSystem::States::PRESSED; }
[[nodiscard]] inline bool GetKeyUp(Keyboard::Key const key) { return InputSystem::s_keyboardStates[key] == InputSystem::States::UP; }
[[nodiscard]] inline bool GetKeyDown(Keyboard::Key const key) { return InputSystem::s_keyboardStates[key] == InputSystem::States::DOWN; }