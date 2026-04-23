#include "pch.h"
#include "InputSystem.h"
#include <map>

inline static std::map<Keyboard::Key, int> s_keyboardMap = {
    { Keyboard::LEFT,            VK_LEFT     },
    { Keyboard::UP,              VK_UP       },
    { Keyboard::RIGHT,           VK_RIGHT    },
    { Keyboard::DOWN,            VK_DOWN     },
    { Keyboard::SHIFT,           VK_LSHIFT   },
    { Keyboard::CONTROL,         VK_LCONTROL },
    { Keyboard::SPACE,           VK_SPACE    },
	{ Keyboard::ESCAPE,          VK_ESCAPE   },
    { Keyboard::ALT,             VK_MENU     },
	{ Keyboard::TAB,             VK_TAB      },
    { Keyboard::A,               'A'         },
    { Keyboard::B,               'B'         },
    { Keyboard::C,               'C'         },
    { Keyboard::D,               'D'         },
    { Keyboard::E,               'E'         },
    { Keyboard::F,               'F'         },
    { Keyboard::G,               'G'         },
    { Keyboard::H,               'H'         },
    { Keyboard::I,               'I'         },
    { Keyboard::J,               'J'         },
    { Keyboard::K,               'K'         },
    { Keyboard::L,               'L'         },
    { Keyboard::M,               'M'         },
    { Keyboard::N,               'N'         },
    { Keyboard::O,               'O'         },
    { Keyboard::P,               'P'         },
    { Keyboard::Q,               'Q'         },
    { Keyboard::R,               'R'         },
    { Keyboard::S,               'S'         },
    { Keyboard::T,               'T'         },
    { Keyboard::U,               'U'         },
    { Keyboard::V,               'V'         },
    { Keyboard::W,               'W'         },
    { Keyboard::X,               'X'         },
    { Keyboard::Y,               'Y'         },
	{ Keyboard::Z,               'Z'         }
};

inline static std::map<Mouse::Button, int> s_mouseMap = {
    { Mouse::MOUSELEFT,     VK_LBUTTON  },
    { Mouse::MOUSERIGHT,    VK_RBUTTON  },
    { Mouse::MIDDLE,        VK_MBUTTON  },
};

void InputSystem::HandleInputs()
{
    s_windowHandle = GetActiveWindow();

    // Keyboard
    for (auto& [key, code] : s_keyboardMap)
    {
        bool pressed = (GetAsyncKeyState(code) & 0x8000) != 0;

        States& state = s_keyboardStates[key];

        bool wasPressed = (state == PRESSED || state == DOWN);

        if (pressed)
            state = wasPressed ? PRESSED : DOWN;
        else
            state = wasPressed ? UP : NONE;
    }

    // Mouse
    for (auto& [button, code] : s_mouseMap)
    {
        bool pressed = (GetAsyncKeyState(code) & 0x8000) != 0;

        States& state = s_mouseStates[button];
        bool wasPressed = (state == PRESSED || state == DOWN);

        state = pressed ? (wasPressed ? PRESSED : DOWN)
            : (wasPressed ? UP : NONE);
    }
}
