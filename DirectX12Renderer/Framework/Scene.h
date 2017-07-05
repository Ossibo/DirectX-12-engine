#ifndef _GRAPHICSCLASS_H_
#define _GRAPHICSCLASS_H_


//////////////
// INCLUDES //
//////////////

///////////////////////
// MY CLASS INCLUDES //
///////////////////////
#include "Graphics/Direct3DManager.h"
#include "Graphics/Shaders/SimpleShader.h"
#include "Graphics/Objects/Model.h"
#include "DirectX.h"
#include "Graphics/Camera.h"
#include "InputHandler.h"

#include <cmath>

/////////////
// GLOBALS //
/////////////

#define WINDOW_SCREEN_WIDTH 1920
#define WINDOW_SCREEN_HEIGT 1080

const bool FULL_SCREEN = false;
const bool VSYNC_ENABLED = false;
const bool SHOW_CURSER = false;
const float SCREEN_DEPTH = 500.0f;
const float SCREEN_NEAR = 0.1f;

class Scene
{
public:
	Scene();
	Scene(const Scene& other);
	~Scene();

	bool Initialize(int screenHeight, int screenWidth, HWND hwnd, InputHandler* input);
	bool Frame(const float& dt);

private:
	bool Render();
	bool TestEnviormentForHowThingsWork();

private:
	Direct3DManager* m_direct3D;
	InputHandler* m_input;
	Model* m_model;
	Model* m_floor;
	Model* m_oildrum;
	SimpleShader* m_simpleShader;
	Camera* m_camera;

	int m_currentModelForDisplay;

};

#endif