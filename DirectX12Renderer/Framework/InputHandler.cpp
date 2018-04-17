#include "InputHandler.h"

InputHandler::InputHandler()
{
	m_directInput = 0;
	m_keyboard = 0;
	m_mouse = 0;

	for (int i = 0; i < 256; ++i)
	{
		m_activeState[i] = false;
	}
}

InputHandler::InputHandler(const InputHandler& other)
{	
}

InputHandler::~InputHandler()
{
	SAFE_RELEASE_INPUT(m_mouse);
	SAFE_RELEASE_INPUT(m_keyboard);
	SAFE_RELEASE(m_directInput);
}

bool InputHandler::Initialize(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight)
{
	HRESULT result;

	// Store the screen size which will be used for positioning the mouse cursor.
	m_screenWidth = screenWidth;
	m_screenHeight = screenHeight;

	// Initialize the location of the mouse on the screen.
	m_mouseX = 0;
	m_mouseY = 0;

	// Initialize the main direct input interface
	result = DirectInput8Create(hinstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&m_directInput, NULL);
	FAIL(result);

	// Initialize the direct input interface for the keyboard
	result = m_directInput->CreateDevice(GUID_SysKeyboard, &m_keyboard, NULL);
	FAIL(result);

	// Set the data format.  In this case since it is a keyboard we can use the predefined data format.
	result = m_keyboard->SetDataFormat(&c_dfDIKeyboard);
	FAIL(result);

	// Set the cooperative level of the keyboard to not share with other programs
	result = m_keyboard->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_EXCLUSIVE);
	FAIL(result);

	// Now acquire the keyboard
	result = m_keyboard->Acquire();
	FAIL(result);

	// Initialize the direct interface for mouse
	result = m_directInput->CreateDevice(GUID_SysMouse, &m_mouse, NULL);
	FAIL(result);

	// Set the data format for the mouse using the pre-defined mouse data format
	result = m_mouse->SetDataFormat(&c_dfDIMouse);
	FAIL(result);

	// Set the cooperative level of the mouse to share with other programs
	result = m_mouse->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
	FAIL(result);

	// Acquire the mouse
	result = m_mouse->Acquire();
	FAIL(result);

	return true;
}

bool InputHandler::Frame()
{
	bool result;

	// Read the current state of the keyboard.
	result = ReadKeyboard();
	ASSERT(result);

	// Read the current state of the mouse.
	result = ReadMouse();
	ASSERT(result);

	// Process the changes in the mouse and keyboard.
	ProcessInput();

	return true;
}

void InputHandler::GetMouseLocation(int& mouseX, int& mouseY)
{
	mouseX = m_mouseX;
	mouseY = m_mouseY;
}

bool InputHandler::IsKeyPressed(Keys key) {
	if (m_keyboardState[key] & 0x80 && !m_activeState[key])
	{
		m_activeState[key] = true;
	}
	else if (!(m_keyboardState[key] & 0x80) && m_activeState[key])
	{
		m_activeState[key] = false;
		return true;
	}

	return false;
}

bool InputHandler::IsKeyDown(Keys key) {
	if (m_keyboardState[key] & 0x80) {
		return true;
	}
	return false;
}

bool InputHandler::ReadKeyboard()
{
	HRESULT result;

	// Read the keyboard device.
	result = m_keyboard->GetDeviceState(sizeof(m_keyboardState), (LPVOID)&m_keyboardState);
	if (FAILED(result))
	{
		// If the keyboard lost focus or was not acquired then try to get control back.
		if ((result == DIERR_INPUTLOST) || (result == DIERR_NOTACQUIRED))
		{
			m_keyboard->Acquire();
		}
		else
		{
			return false;
		}
	}

	return true;
}

bool InputHandler::ReadMouse()
{
	HRESULT result;

	// Read the mouse device.
	result = m_mouse->GetDeviceState(sizeof(DIMOUSESTATE), (LPVOID)&m_mouseState);
	if (FAILED(result))
	{
		// If the mouse lost focus or was not acquired then try to get control back.
		if ((result == DIERR_INPUTLOST) || (result == DIERR_NOTACQUIRED))
		{
			m_mouse->Acquire();
		}
		else
		{
			return false;
		}
	}

	return true;
}

void InputHandler::ProcessInput()
{
	// Update the location of the mouse cursor based on the change of the mouse location during the frame.
	m_mouseX += m_mouseState.lX;
	m_mouseY += m_mouseState.lY;

	// Ensure the mouse location doesn't exceed the screen width or height.
	if (m_mouseX < 0) { m_mouseX = 0; }
	if (m_mouseY < 0) { m_mouseY = 0; }

	if (m_mouseX > m_screenWidth) { m_mouseX = m_screenWidth; }
	if (m_mouseY > m_screenHeight) { m_mouseY = m_screenHeight; }
}
