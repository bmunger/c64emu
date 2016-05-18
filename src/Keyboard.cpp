#include "Keyboard.h"

Keyboard::Keyboard()
{
	for (int i = 0; i < 8; i++)
	{
		KeysDown[i] = 0;
	}
}

void Keyboard::KeyEvent(SDL_KeyboardEvent& keyEvent)
{
	if (keyEvent.type == SDL_KEYDOWN || keyEvent.type == SDL_KEYUP)
	{
		C64KeyMap key = C64Key_C64;
		bool foundKey = true;

		switch (keyEvent.keysym.scancode)
		{
		case SDL_SCANCODE_0: key = C64Key_0; break;
		case SDL_SCANCODE_1: key = C64Key_1; break;
		case SDL_SCANCODE_2: key = C64Key_2; break;
		case SDL_SCANCODE_3: key = C64Key_3; break;
		case SDL_SCANCODE_4: key = C64Key_4; break;
		case SDL_SCANCODE_5: key = C64Key_5; break;
		case SDL_SCANCODE_6: key = C64Key_6; break;
		case SDL_SCANCODE_7: key = C64Key_7; break;
		case SDL_SCANCODE_8: key = C64Key_8; break;
		case SDL_SCANCODE_9: key = C64Key_9; break;

		case SDL_SCANCODE_A: key = C64Key_A; break;
		case SDL_SCANCODE_B: key = C64Key_B; break;
		case SDL_SCANCODE_C: key = C64Key_C; break;
		case SDL_SCANCODE_D: key = C64Key_D; break;
		case SDL_SCANCODE_E: key = C64Key_E; break;
		case SDL_SCANCODE_F: key = C64Key_F; break;
		case SDL_SCANCODE_G: key = C64Key_G; break;
		case SDL_SCANCODE_H: key = C64Key_H; break;
		case SDL_SCANCODE_I: key = C64Key_I; break;
		case SDL_SCANCODE_J: key = C64Key_J; break;
		case SDL_SCANCODE_K: key = C64Key_K; break;
		case SDL_SCANCODE_L: key = C64Key_L; break;
		case SDL_SCANCODE_M: key = C64Key_M; break;
		case SDL_SCANCODE_N: key = C64Key_N; break;
		case SDL_SCANCODE_O: key = C64Key_O; break;
		case SDL_SCANCODE_P: key = C64Key_P; break;
		case SDL_SCANCODE_Q: key = C64Key_Q; break;
		case SDL_SCANCODE_R: key = C64Key_R; break;
		case SDL_SCANCODE_S: key = C64Key_S; break;
		case SDL_SCANCODE_T: key = C64Key_T; break;
		case SDL_SCANCODE_U: key = C64Key_U; break;
		case SDL_SCANCODE_V: key = C64Key_V; break;
		case SDL_SCANCODE_W: key = C64Key_W; break;
		case SDL_SCANCODE_X: key = C64Key_X; break;
		case SDL_SCANCODE_Y: key = C64Key_Y; break;
		case SDL_SCANCODE_Z: key = C64Key_Z; break;

		case SDL_SCANCODE_BACKSPACE: key = C64Key_Delete; break;
		case SDL_SCANCODE_KP_PLUS: key = C64Key_Plus; break;
		case SDL_SCANCODE_RETURN: key = C64Key_Return; break;
		case SDL_SCANCODE_KP_MULTIPLY: key = C64Key_Asterisk; break;
		case SDL_SCANCODE_ESCAPE: key = C64Key_BackArrow; break;
		case SDL_SCANCODE_RIGHT: key = C64Key_CursorRt; break;
		case SDL_SCANCODE_SEMICOLON: key = C64Key_Colon; break; // Intentional break
		case SDL_SCANCODE_LCTRL: key = C64Key_Ctrl; break;
		case SDL_SCANCODE_F7: key = C64Key_F7; break;
		case SDL_SCANCODE_MINUS: key = C64Key_Minus; break;
		case SDL_SCANCODE_HOME: key = C64Key_Home; break;
		case SDL_SCANCODE_F1: key = C64Key_F1; break;
		case SDL_SCANCODE_PERIOD: key = C64Key_Period; break;
		case SDL_SCANCODE_RSHIFT: key = C64Key_RShift; break;
		case SDL_SCANCODE_SPACE: key = C64Key_Space; break;
		case SDL_SCANCODE_F3: key = C64Key_F3; break;
		case SDL_SCANCODE_APOSTROPHE: key = C64Key_Semicolon; break; // Wow the keyboard layout is different.
		case SDL_SCANCODE_EQUALS: key = C64Key_Equals; break;
		case SDL_SCANCODE_PAGEUP: key = C64Key_C64; break; // C64 logo: page up for now.
		case SDL_SCANCODE_F5: key = C64Key_F5; break;
		case SDL_SCANCODE_PAGEDOWN: key = C64Key_Ampersand; break; // Ampersand is page down.
		case SDL_SCANCODE_GRAVE: key = C64Key_UpArrow; break; // The mysterious up arrow is mapped to backquote/grave
		case SDL_SCANCODE_DOWN: key = C64Key_CursorDn; break;
		case SDL_SCANCODE_LSHIFT: key = C64Key_LShift; break;
		case SDL_SCANCODE_COMMA: key = C64Key_Comma; break;
		case SDL_SCANCODE_SLASH: key = C64Key_Slash; break;
		case SDL_SCANCODE_END: key = C64Key_Stop; break; // End is mapped to Stop


		// (None of the keys are unmapped, but the above mappings are far from perfect...)

		default:
			foundKey = false;
			break;
		}

		if (foundKey)
		{
			if (keyEvent.type == SDL_KEYDOWN)
			{
				KeyDown64(key);
			}
			else
			{
				KeyUp64(key);
			}
		}
	}
}

void Keyboard::KeyDown64(C64KeyMap key)
{
	int arrayIndex = key & 7;
	int arrayBit = (key >> 3) & 7;
	KeysDown[arrayIndex] |= 1 << arrayBit;
}
void Keyboard::KeyUp64(C64KeyMap key)
{
	int arrayIndex = key & 7;
	int arrayBit = (key >> 3) & 7;
	KeysDown[arrayIndex] &= ~(1 << arrayBit);
}

void Keyboard::UpdateKeyboardMatrix(unsigned char & PRA, unsigned char & PRB, unsigned char DDRA, unsigned char DDRB)
{
	// Figure out if we should modify the port registers based on keyboard state.
	// I'm making the assumption that the C64 keyboard has diodes;
	//   So any bit in port A that's driven to 0 will lead to corresponding bits in port B being pulled to 0 if the corresponding key is pressed.
	//   We'll see if this turns out to be correct.

	unsigned char drivenBits = PRA | (~DDRA);
	unsigned char matrixOutput = 0xFF;

	for (int i = 0; i < 8; i++)
	{
		if ((drivenBits & (1 << i)) == 0)
		{
			matrixOutput &= ~(KeysDown[i]);
		}
	}

	// For any inputs in Port B, overwrite the bits with matrixOutput
	PRB = (PRB & DDRB) | ((~DDRB) & matrixOutput);
}