#pragma once

struct Mouse { enum Button : int {
	MOUSELEFT,
	MOUSERIGHT,
	MIDDLE,

	AMOUNT,
};
};

struct Keyboard { enum Key : int {
	LEFT,
	RIGHT,
	UP,
	DOWN,

	A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z, 

	SHIFT,
	CONTROL,
	SPACE,

	ALT,
	TAB,
	ESCAPE,

	AMOUNT,
};
};
