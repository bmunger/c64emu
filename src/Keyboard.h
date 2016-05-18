#ifndef _KEYBOARD_H
#define _KEYBOARD_H

#include "c64emu.h"


// Map of key codes to C64 key matrix location. 
// Bottom 3 bits are Port A matrix location, next 3 bits are Port B matrix location.
enum C64KeyMap
{
	C64Key_Delete = 0,
	C64Key_3 = 1,
	C64Key_5 = 2,
	C64Key_7 = 3,
	C64Key_9 = 4,
	C64Key_Plus = 5,
	C64Key_Pound = 6,
	C64Key_1 = 7,

	C64Key_Return = 0 + (1<<3),
	C64Key_W = 1 + (1 << 3),
	C64Key_R = 2 + (1 << 3),
	C64Key_Y = 3 + (1 << 3),
	C64Key_I = 4 + (1 << 3),
	C64Key_P = 5 + (1 << 3),
	C64Key_Asterisk = 6 + (1 << 3),
	C64Key_BackArrow = 7 + (1 << 3),

	C64Key_CursorRt = 0 + (2 << 3),
	C64Key_A = 1 + (2 << 3),
	C64Key_D = 2 + (2 << 3),
	C64Key_G = 3 + (2 << 3),
	C64Key_J = 4 + (2 << 3),
	C64Key_L = 5 + (2 << 3),
	C64Key_Semicolon = 6 + (2 << 3),
	C64Key_Ctrl = 7 + (2 << 3),

	C64Key_F7 = 0 + (3 << 3),
	C64Key_4 = 1 + (3 << 3),
	C64Key_6 = 2 + (3 << 3),
	C64Key_8 = 3 + (3 << 3),
	C64Key_0 = 4 + (3 << 3),
	C64Key_Minus = 5 + (3 << 3),
	C64Key_Home = 6 + (3 << 3),
	C64Key_2 = 7 + (3 << 3),

	C64Key_F1 = 0 + (4 << 3),
	C64Key_Z = 1 + (4 << 3),
	C64Key_C = 2 + (4 << 3),
	C64Key_B = 3 + (4 << 3),
	C64Key_M = 4 + (4 << 3),
	C64Key_Period = 5 + (4 << 3),
	C64Key_RShift = 6 + (4 << 3),
	C64Key_Space = 7 + (4 << 3),

	C64Key_F3 = 0 + (5 << 3),
	C64Key_S = 1 + (5 << 3),
	C64Key_F = 2 + (5 << 3),
	C64Key_H = 3 + (5 << 3),
	C64Key_K = 4 + (5 << 3),
	C64Key_Colon = 5 + (5 << 3),
	C64Key_Equals = 6 + (5 << 3),
	C64Key_C64 = 7 + (5 << 3),

	C64Key_F5 = 0 + (6 << 3),
	C64Key_E = 1 + (6 << 3),
	C64Key_T = 2 + (6 << 3),
	C64Key_U = 3 + (6 << 3),
	C64Key_O = 4 + (6 << 3),
	C64Key_Ampersand = 5 + (6 << 3),
	C64Key_UpArrow = 6 + (6 << 3),
	C64Key_Q = 7 + (6 << 3),

	C64Key_CursorDn = 0 + (7 << 3),
	C64Key_LShift = 1 + (7 << 3),
	C64Key_X = 2 + (7 << 3),
	C64Key_V = 3 + (7 << 3),
	C64Key_N = 4 + (7 << 3),
	C64Key_Comma = 5 + (7 << 3),
	C64Key_Slash = 6 + (7 << 3),
	C64Key_Stop = 7 + (7 << 3),

};


class Keyboard
{
public:
	Keyboard();

	void KeyEvent(SDL_KeyboardEvent& keyEvent);

	void KeyDown64(C64KeyMap key);
	void KeyUp64(C64KeyMap key);

	void UpdateKeyboardMatrix(unsigned char & PRA, unsigned char & PRB, unsigned char DDRA, unsigned char DDRB);

	// Stores a bitmap of which keys are currently held down.
	// Array index is the port A index, the bits are the keys that are pressed.
	// Here, the bit being set means it's pressed, but when the C64 reads this, it writes a '0' to the Port A line it wants to test, and port B will be '0' for lines where a key is pressed.
	unsigned char KeysDown[8];
};

#endif
