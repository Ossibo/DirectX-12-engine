#include "System.h"


System::System()
{
	m_input = 0;
	m_scene = 0;
}

System::~System()
{
	SAFE_DELETE(m_input);
	SAFE_DELETE(m_scene);
	ShutdownWindows();
}

bool System::Initialize()
{
	int screenHeight, screenWidth;
	bool result;

	screenHeight = 0;
	screenWidth = 0;

	InitializeWindows(screenHeight, screenWidth);

	//Create the input object
	m_input = new InputHandler();
	if (!m_input)
	{
		MessageBox(m_hwnd, L"Could not initialize the input object.", L"Error", MB_OK);
		return false;
	}

	//Initialize the Input object
	result = m_input->Initialize(m_hinstance, m_hwnd, screenWidth, screenHeight);
	ASSERT(result);

	//Create the scene object
	m_scene = new Scene();
	if (!m_scene)
	{
		MessageBox(m_hwnd, L"Could not create m_scene.", L"Error", MB_OK);
		return false;
	}

	//Initialize the scene object
	result = m_scene->Initialize(screenHeight, screenWidth, m_hwnd, m_input);
	ASSERT(result);

	return true;
}

void System::Run()
{
	MSG msg;
	bool done, result;

	// For delta time
	__int64 cntsPerSec = 0;
	QueryPerformanceFrequency((LARGE_INTEGER*)&cntsPerSec);
	float secsPerCnt = 1.0f / (float)cntsPerSec;

	__int64 prevTimeStamp = 0;
	QueryPerformanceCounter((LARGE_INTEGER*)&prevTimeStamp);

	//Initialize the message structure
	ZeroMemory(&msg, sizeof(MSG));

	//Loop until there is a quit message from the window of the user.
	done = false;
	while (!done)
	{
		//Handle the windows messages
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		//if windows signals to end the application then exit out.
		if (msg.message == WM_QUIT)
		{
			done = true;
		}
		else
		{
			// Otherwise do the frame processing.
			__int64 currTimeStamp = 0;
			QueryPerformanceCounter((LARGE_INTEGER*)&currTimeStamp);
			float dt = (currTimeStamp - prevTimeStamp) * secsPerCnt;

			result = Frame(dt);
			if (!result)
			{
				done = true;
			}

			prevTimeStamp = currTimeStamp;
		}

		if (m_input->IsKeyPressed(Keys::ESC))
			done = true;
	} 
}

bool System::Frame(const float& dt)
{
	bool result;
	
	//Check if the user pressed escape and wants to exit the application.
	result = m_input->Frame();
	ASSERT(result);

	//Do the frame processing for the scene object
	result = m_scene->Frame(dt);
	ASSERT(result);

	return true;
}

LRESULT CALLBACK System::MessageHandler(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam)
{
	return DefWindowProc(hwnd, umsg, wparam, lparam);
}

void System::InitializeWindows(int& screenHeight, int& screenWidth)
{
	WNDCLASSEX wc;
	DEVMODE dmScreenSettings;
	int posX, posY;

	//Get an external pointer to this object
	ApplicationHandle = this;
	
	//Get the instance of this application
	m_hinstance = GetModuleHandle(NULL);
	
	//Give the application a name
	m_applicationName = L"DirectX12 - Particle Systems";

	wc.style			= CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc		= WndProc;
	wc.cbClsExtra		= 0;
	wc.cbWndExtra		= 0;
	wc.hInstance		= m_hinstance;
	wc.hIcon			= LoadIcon(NULL, IDI_WINLOGO);
	wc.hIconSm			= wc.hIcon;
	wc.hCursor			= LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground	= (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.lpszMenuName		= NULL;
	wc.lpszClassName	= m_applicationName;
	wc.cbSize			= sizeof(WNDCLASSEX);

	//Register the window class
	RegisterClassEx(&wc);

	//Determin the resolution of the clients desktop screen
	screenHeight = GetSystemMetrics(SM_CYSCREEN);
	screenWidth = GetSystemMetrics(SM_CXSCREEN);

	//Setup the screen settings depending on whether it is running in fullscreen of in windowed mode
	if (FULL_SCREEN)
	{
		//if full screen set the screen to maximum size of the users desktop and 32bit
		memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
		dmScreenSettings.dmSize = sizeof(dmScreenSettings);
		dmScreenSettings.dmPelsHeight = (unsigned long)screenHeight;
		dmScreenSettings.dmPelsWidth = (unsigned long)screenWidth;
		dmScreenSettings.dmBitsPerPel = 32;
		dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		//Change the display settings to full screen
		ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN);

		//Set the position of the window to the top left corner
		posX = posY = 0;
	}
	else
	{
		//if windowed then set it to 1280x720
		screenWidth = WINDOW_SCREEN_WIDTH;
		screenHeight = WINDOW_SCREEN_HEIGT;

		//Place the window in the middle of the screen
		posX = (GetSystemMetrics(SM_CXSCREEN) - screenWidth) * 0.5f;
		posY = (GetSystemMetrics(SM_CYSCREEN) - screenHeight) * 0.5f;
	}

	//Create hte window with the screen settings and get the handle to it
	m_hwnd = CreateWindowEx(WS_EX_APPWINDOW, m_applicationName, m_applicationName, WS_OVERLAPPED | WS_SYSMENU | WS_VISIBLE | WS_CAPTION | WS_MINIMIZEBOX, posX, posY, screenWidth, screenHeight, NULL, NULL, m_hinstance, NULL);

	//Bring the window up on screen and set it as main focus
	ShowWindow(m_hwnd, SW_SHOW);
	SetForegroundWindow(m_hwnd);
	SetFocus(m_hwnd);

	ShowCursor(SHOW_CURSER);
}

void System::ShutdownWindows()
{
	//Show the mouse cursor
	ShowCursor(true);

	//Fix the display settings of leaving full screen mode
	if (FULL_SCREEN)
	{
		ChangeDisplaySettings(NULL, 0);
	}
	//Remove the Window
	DestroyWindow(m_hwnd);
	m_hwnd = NULL;

	//Remove the application instance
	UnregisterClass(m_applicationName, m_hinstance);
	m_hinstance = NULL;

	//RELEASE the pointer to this class
	ApplicationHandle = NULL;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam)
{
	switch (umessage)
	{
		// Check if the window is being destroyed.
	case WM_DESTROY:
	{
		PostQuitMessage(0);
		return 0;
	}

	// Check if the window is being closed.
	case WM_CLOSE:
	{
		PostQuitMessage(0);
		return 0;
	}

	// All other messages pass to the message handler in the system class.
	default:
	{
		return ApplicationHandle->MessageHandler(hwnd, umessage, wparam, lparam);
	}
	}
}