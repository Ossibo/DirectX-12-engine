#ifndef _INPUTHANDLER_H_
#define _INPUTHANDLER_H_

///////////////////////////////
// PRE-PROCESSING DIRECTIVES //
///////////////////////////////
#define DIRECTINPUT_VERSION 0x0800

/////////////
// LINKING //
/////////////
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

//////////////
// INCLUDES //
//////////////
#include <dinput.h>

///////////////////////
// MY CLASS INCLUDES //
///////////////////////
#include "DirectX.h"

enum Keys {
	Left = DIK_LEFT,
	Right = DIK_RIGHT,
	Up = DIK_UP,
	Down = DIK_DOWN,
	One = DIK_1,
	Two = DIK_2,
	Three = DIK_3,
	Four = DIK_4,
	Five = DIK_5,
	W = DIK_W,
	A = DIK_A,
	S = DIK_S,
	D = DIK_D,
	E = DIK_E,
	Space = DIK_SPACE,
	Enter = DIK_RETURN,
	CTRL = DIK_LCONTROL,
	ESC = DIK_ESCAPE
};

class InputHandler
{
public:
	InputHandler();
	InputHandler(const InputHandler&);
	~InputHandler();

	bool Initialize(HINSTANCE, HWND, int, int);
	bool Frame();

	bool IsKeyPressed(Keys);
	bool IsKeyDown(Keys);
	void GetMouseLocation(int&, int&);

	float GetMouseDeltaX() { return m_mouseState.lX; }
	float GetMouseDeltaY() { return m_mouseState.lY; }

private:
	bool ReadKeyboard();
	bool ReadMouse();
	void ProcessInput();

private:
	IDirectInput8* m_directInput;
	IDirectInputDevice8* m_keyboard;
	IDirectInputDevice8* m_mouse;

	// Current states
	bool m_activeState[256];
	unsigned char m_keyboardState[256];
	DIMOUSESTATE m_mouseState;
	int m_screenWidth, m_screenHeight;
	int m_mouseX, m_mouseY;
};

#endif