#include "Scene.h"

Scene::Scene()
{
	m_direct3D = 0;
	m_model = 0;
	m_floor = 0;
	m_simpleShader = 0;
	m_camera = 0;
	m_oildrum = 0;
	m_currentModelForDisplay = 1;
}

Scene::~Scene()
{
	SAFE_DELETE(m_direct3D);
	SAFE_DELETE(m_model);
	SAFE_DELETE(m_floor);
	SAFE_DELETE(m_simpleShader);
	SAFE_DELETE(m_camera);
	SAFE_DELETE(m_oildrum);
}

bool Scene::Initialize(int screenHeight, int screenWidth, HWND hwnd, InputHandler* input)
{
	bool result;

	//Create the Direct3D object
	m_direct3D = new Direct3DManager();

	result = m_direct3D->Initialize(screenHeight, screenWidth, hwnd, VSYNC_ENABLED, FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR);// , SCREEN_DEPTH, SCREEN_NEAR);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize Direct3D", L"Error", MB_OK);
		return false;
	}

	// Create input handler
	m_input = input;

	//Create simple shader
	m_simpleShader = new SimpleShader();

	result = m_simpleShader->Initialize(m_direct3D, hwnd);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize Simple Shader Object", L"Error", MB_OK);
		return false;
	}

	//Create model but we want a dynamic way to load the models instead
	m_model = new Model();
	ASSERT(m_model->Initialize(m_direct3D, "Assets/Objects/Gun/ak.obj"));
	m_model->SetRotation(XMFLOAT3(0.0f, 0.0f, 0.0f));
	m_model->SetPosition(XMFLOAT3(0.0f, 0.0f, 0.0f));
	m_model->SetScale(XMFLOAT3(1.0f, 1.0f, 1.0f));

	m_floor = new Model();
	ASSERT(m_floor->Initialize(m_direct3D, "Assets/Objects/Floor/Floor.obj"));
	m_floor->SetPosition(XMFLOAT3(0.0f, -1.0f, 0.0f));
	m_floor->SetPosition(XMFLOAT3(0.0f, 0.0f, 0.0f));
	m_floor->SetScale(XMFLOAT3(1.0f, 1.0f, 1.0f));

	m_oildrum = new Model();
	ASSERT(m_oildrum->Initialize(m_direct3D, "Assets/Objects/oildrum/oildrum.obj"));
	m_oildrum->SetPosition(XMFLOAT3(0.0f, -1.0f, 0.0f));
	m_oildrum->SetPosition(XMFLOAT3(0.0f, 0.0f, 0.0f));
	m_oildrum->SetScale(XMFLOAT3(3.0f, 3.0f, 3.0f));


	m_direct3D->GetCommandList()->Close();
	ID3D12CommandList* ppCommandLists[] = { m_direct3D->GetCommandList() };
	m_direct3D->GetCommandQueue()->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	m_direct3D->IncrementFence();
	m_direct3D->SignalFence();
	
	//Create camera
	m_camera = new Camera();

	m_camera->SetPosition(0.0f, 6.0f, -15.0f);
	m_camera->SetRotation(20.0f, 0.0f, 0.0f);
	m_camera->SetMovementSpeed(5.0f);
	return true;
}
void Scene::InputAlterations(const float& dt)
{
	// Add camera rotation
	m_camera->AddRotation(m_input->GetMouseDeltaX() / 10.f, m_input->GetMouseDeltaY() / 10.f);
	m_camera->Update(dt);

	// forward camera movement
	if (m_input->IsKeyDown(Keys::W))
		m_camera->MoveForward(dt);
	if (m_input->IsKeyDown(Keys::S))
		m_camera->MoveForward(dt, -1);

	// Strafe camera movement
	if (m_input->IsKeyDown(Keys::D))
		m_camera->Strafe(dt);
	else if (m_input->IsKeyDown(Keys::A))
		m_camera->Strafe(dt, -1);

	// Vertical camera movement
	if (m_input->IsKeyDown(Keys::Space))
		m_camera->MoveVertical(dt);
	else if (m_input->IsKeyDown(Keys::CTRL))
		m_camera->MoveVertical(dt, -1);

	if (m_input->IsKeyPressed(Keys::One))
		m_currentModelForDisplay = 1;
	if (m_input->IsKeyPressed(Keys::Two))
		m_currentModelForDisplay = 2;
	if (m_input->IsKeyPressed(Keys::Three))
		m_currentModelForDisplay = 3;
	if (m_input->IsKeyPressed(Keys::Four))
		m_currentModelForDisplay = 4;

	if (m_input->IsKeyPressed(Keys::E))
		m_simpleShader->ChangeWireFrame();
}
void Scene::UpdateModels(const float& dt)
{
	//ROTATE MODEL
	m_model->SetRotation(XMFLOAT3(0.0f, m_model->GetRotation().y + dt * 0.2f, 0.0f));
	m_oildrum->SetRotation(XMFLOAT3(0.0f, m_oildrum->GetRotation().y + dt * -0.2f, 0.0f));
}
bool Scene::Frame(const float& dt)
{
	bool result;

	m_camera->Render();

	//Render the Graphics scene
	result = Render();
	ASSERT(result);

	InputAlterations(dt);
	UpdateModels(dt);

	return true;
}

bool Scene::Render()
{
	bool result;

	//Use the Direct3D object to render the scene
	result = m_direct3D->BeginScene(0.0f, 0.2f, 0.4f, 1.0f);
	ASSERT(result);
	
	// With the 1-4 on the keyboard to choose what should be displayed
	switch (m_currentModelForDisplay)
	{
	case 1:
		m_model->Render(m_direct3D, m_simpleShader, m_camera);
		m_floor->Render(m_direct3D, m_simpleShader, m_camera);
		break;

	case 2:
		m_oildrum->Render(m_direct3D, m_simpleShader, m_camera);
		break;

	case 3:
		m_floor->Render(m_direct3D, m_simpleShader, m_camera);
		break;
	case 4:
		m_floor->Render(m_direct3D, m_simpleShader, m_camera);
		m_oildrum->Render(m_direct3D, m_simpleShader, m_camera);
		m_model->Render(m_direct3D, m_simpleShader, m_camera);
		break;

	default:
		break;
	}

	result = m_direct3D->EndScene();
	ASSERT(result);

	return true;
}