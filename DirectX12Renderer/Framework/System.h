#ifndef _SYSTEM_H_
#define _SYSTEM_H_

///////////////////////////////
// PRE-PROCESSING DIRECTIVES //
///////////////////////////////
#define WIN32_LEAN_AND_MEAN

//////////////
// INCLUDES //
//////////////
#include <windows.h>

///////////////////////
// MY CLASS INCLUDES //
///////////////////////
#include "InputHandler.h"
#include "Scene.h"
#include "DirectX.h"

class System
{
public:
	System();
	System(const System& other);
	~System();
	bool Initialize();
	void Run();

	LRESULT CALLBACK MessageHandler(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam);

private:
	bool Frame(const float& dt);
	void InitializeWindows(int& screenHeight, int& screenWidth);
	void ShutdownWindows();

private:
	LPCWSTR m_applicationName;
	HINSTANCE m_hinstance;
	HWND m_hwnd;

	InputHandler* m_input;
	Scene* m_scene;
};

/////////////////////////
// FUNCTION PROTOTYPES //
/////////////////////////
static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

/////////////
// GLOBALS //
/////////////
static System* ApplicationHandle = 0;

#endif