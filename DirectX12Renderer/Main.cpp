#include "Framework/System.h"

int WINAPI WinMain(HINSTANCE HInstance, HINSTANCE hPrevInstance, PSTR pScmdline, int iCmdshow)
{
	System* system;
	bool result;

	//Create the system object.
	system = new System();
	if (!system)
	{
		return 0;
	}
	result = system->Initialize();
	if (result)
	{
		system->Run();
	}
	delete system;
	system = 0;

	return 0;
}