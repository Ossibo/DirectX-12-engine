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

Scene::Scene(const Scene& other)
{

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

	//Create model
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
	result = m_direct3D->SignalFence();
	FAIL(result);
	
	//Create camera
	m_camera = new Camera();

	m_camera->SetPosition(0.0f, 6.0f, -15.0f);
	m_camera->SetRotation(20.0f, 0.0f, 0.0f);
	m_camera->SetMovementSpeed(5.0f);
	return true;
}

bool Scene::Frame(const float& dt)
{
	bool result;
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

	m_camera->Render();

	if (m_input->IsKeyPressed(Keys::One))
		m_currentModelForDisplay = 1;
	if (m_input->IsKeyPressed(Keys::Two))
		m_currentModelForDisplay = 2;
	if (m_input->IsKeyPressed(Keys::Three))
		m_currentModelForDisplay = 3;

	if (m_input->IsKeyPressed(Keys::E))
		m_simpleShader->ChangeWireFrame();




	//Render hte Graphics scene
	result = Render();
	ASSERT(result);

	//ROTATE MODEL
	m_model->SetRotation(XMFLOAT3(0.0f, m_model->GetRotation().y + dt * 0.2f, 0.0f));
	m_oildrum->SetRotation(XMFLOAT3(0.0f, m_oildrum->GetRotation().y + dt * -0.2f, 0.0f));

	//MOVE MODEL
	//m_model->SetPosition(XMFLOAT3(0.0f, m_model->GetPosition().y + dt, 0.0f));

	return true;
}

bool Scene::Render()
{
	bool result;

	//Use the Direct3D object to render the scene
	result = m_direct3D->BeginScene(0.0f, 0.2f, 0.4f, 1.0f);
	ASSERT(result);
	ID3D12GraphicsCommandList* commandList = m_direct3D->GetCommandList();

	m_model->m_worldMatrix = XMMatrixScaling(m_model->m_scale.x, m_model->m_scale.y, m_model->m_scale.z) * XMMatrixRotationX(m_model->m_rotation.x) * XMMatrixRotationY(m_model->m_rotation.y) * XMMatrixRotationZ(m_model->m_rotation.z) * XMMatrixTranslation(m_model->m_position.x, m_model->m_position.y, m_model->m_position.z);

	///////////
	//M_FLOOR//
	///////////
	m_floor->m_worldMatrix = XMMatrixScaling(m_floor->m_scale.x, m_floor->m_scale.y, m_floor->m_scale.z) * XMMatrixRotationX(m_floor->m_rotation.x) * XMMatrixRotationY(m_floor->m_rotation.y) * XMMatrixRotationZ(m_floor->m_rotation.z) * XMMatrixTranslation(m_floor->m_position.x, m_floor->m_position.y, m_floor->m_position.z);

	for (int i = 0; i < m_floor->m_meshes.size(); ++i)
	{
		if (commandList->Reset(m_direct3D->m_commandAllocator[m_direct3D->m_bufferIndex], m_direct3D->m_pipelineObject) == S_OK)
		{
			commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_direct3D->m_backBufferRenderTarget[m_direct3D->m_bufferIndex], D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));
			CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_direct3D->m_renderTargetViewHeap->GetCPUDescriptorHandleForHeapStart(), m_direct3D->m_bufferIndex, m_direct3D->m_renderTargetViewDescriptorSize);
			CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(m_direct3D->m_depthStencilViewHeap->GetCPUDescriptorHandleForHeapStart());

			commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);
		}

		commandList->RSSetViewports(1, &m_direct3D->m_viewport);
		commandList->RSSetScissorRects(1, &m_direct3D->m_scissorRect);
		commandList->SetPipelineState(m_simpleShader->m_pipelineState);
		commandList->SetGraphicsRootSignature(m_simpleShader->m_rootSignature);
		MyMesh* mesh = m_floor->m_meshes[i];
		MyMaterial* material = m_floor->m_materials[mesh->m_materialIndex];

		if (material->m_diffuseTexture)
			material->m_diffuseTexture->UploadTexture(m_direct3D);

		if (material->m_normalTexture)
			material->m_normalTexture->UploadTexture(m_direct3D);

		if (material->m_specularTexture)
			material->m_specularTexture->UploadTexture(m_direct3D);

		commandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		commandList->IASetVertexBuffers(0, 1, &mesh->m_vertexBufferView);
		commandList->IASetIndexBuffer(&mesh->m_indexBufferView);
		m_simpleShader->Render(m_direct3D, m_floor, m_camera);
		commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_direct3D->m_backBufferRenderTarget[m_direct3D->m_bufferIndex], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));
		commandList->Close();
		ID3D12CommandList* ppCommandLists[] = { commandList };
		m_direct3D->m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
	}
	m_direct3D->SignalFence();
	m_direct3D->ResetAllocator();
	///////////
	//M_MODEL//
	///////////
	for (int i = 0; i < m_model->m_meshes.size(); ++i)
	{
		if (commandList->Reset(m_direct3D->m_commandAllocator[m_direct3D->m_bufferIndex], m_direct3D->m_pipelineObject) == S_OK)
		{
			commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_direct3D->m_backBufferRenderTarget[m_direct3D->m_bufferIndex], D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));
			CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_direct3D->m_renderTargetViewHeap->GetCPUDescriptorHandleForHeapStart(), m_direct3D->m_bufferIndex, m_direct3D->m_renderTargetViewDescriptorSize);
			CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(m_direct3D->m_depthStencilViewHeap->GetCPUDescriptorHandleForHeapStart());

			commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);
		}

		commandList->RSSetViewports(1, &m_direct3D->m_viewport);
		commandList->RSSetScissorRects(1, &m_direct3D->m_scissorRect);
		commandList->SetPipelineState(m_simpleShader->m_pipelineState);
		commandList->SetGraphicsRootSignature(m_simpleShader->m_rootSignature);

		MyMesh* mesh = m_model->m_meshes[i];
		MyMaterial* material = m_model->m_materials[mesh->m_materialIndex];

		if (material->m_diffuseTexture)
			material->m_diffuseTexture->UploadTexture(m_direct3D);

		if (material->m_normalTexture)
			material->m_normalTexture->UploadTexture(m_direct3D);

		if (material->m_specularTexture)
			material->m_specularTexture->UploadTexture(m_direct3D);

		commandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		commandList->IASetVertexBuffers(0, 1, &mesh->m_vertexBufferView);
		commandList->IASetIndexBuffer(&mesh->m_indexBufferView);
		m_simpleShader->Render(m_direct3D, m_model, m_camera);
		commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_direct3D->m_backBufferRenderTarget[m_direct3D->m_bufferIndex], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));
		commandList->Close();
		ID3D12CommandList* ppCommandLists[] = { commandList };
		m_direct3D->m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
	}
	m_direct3D->SignalFence();
	m_direct3D->ResetAllocator();
	/////////////
	//M_OILDRUM//
	/////////////
	m_oildrum->m_worldMatrix = XMMatrixScaling(m_oildrum->m_scale.x, m_oildrum->m_scale.y, m_oildrum->m_scale.z) * XMMatrixRotationX(m_oildrum->m_rotation.x) * XMMatrixRotationY(m_oildrum->m_rotation.y) * XMMatrixRotationZ(m_oildrum->m_rotation.z) * XMMatrixTranslation(m_oildrum->m_position.x, m_oildrum->m_position.y, m_oildrum->m_position.z);

	for (int i = 0; i < m_oildrum->m_meshes.size(); ++i)
	{
		if (commandList->Reset(m_direct3D->m_commandAllocator[m_direct3D->m_bufferIndex], m_direct3D->m_pipelineObject) == S_OK)
		{
			commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_direct3D->m_backBufferRenderTarget[m_direct3D->m_bufferIndex], D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));
			CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_direct3D->m_renderTargetViewHeap->GetCPUDescriptorHandleForHeapStart(), m_direct3D->m_bufferIndex, m_direct3D->m_renderTargetViewDescriptorSize);
			CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(m_direct3D->m_depthStencilViewHeap->GetCPUDescriptorHandleForHeapStart());

			commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);
		}

		commandList->RSSetViewports(1, &m_direct3D->m_viewport);
		commandList->RSSetScissorRects(1, &m_direct3D->m_scissorRect);
		commandList->SetPipelineState(m_simpleShader->m_pipelineState);
		commandList->SetGraphicsRootSignature(m_simpleShader->m_rootSignature);
		MyMesh* mesh = m_oildrum->m_meshes[i];
		MyMaterial* material = m_oildrum->m_materials[mesh->m_materialIndex];

		if (material->m_diffuseTexture)
			material->m_diffuseTexture->UploadTexture(m_direct3D);

		if (material->m_normalTexture)
			material->m_normalTexture->UploadTexture(m_direct3D);

		if (material->m_specularTexture)
			material->m_specularTexture->UploadTexture(m_direct3D);

		commandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		commandList->IASetVertexBuffers(0, 1, &mesh->m_vertexBufferView);
		commandList->IASetIndexBuffer(&mesh->m_indexBufferView);
		m_simpleShader->Render(m_direct3D, m_oildrum, m_camera);
		commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_direct3D->m_backBufferRenderTarget[m_direct3D->m_bufferIndex], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));
		commandList->Close();
		ID3D12CommandList* ppCommandLists[] = { commandList };
		m_direct3D->m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
	}
	//switch (m_currentModelForDisplay)
	//{
	//case 1:
	//	m_model->Render(m_direct3D, m_simpleShader, m_camera);
	//	m_floor->Render(m_direct3D, m_simpleShader, m_camera);
	//	break;

	//case 2:
	//	m_oildrum->Render(m_direct3D, m_simpleShader, m_camera);
	//	break;

	//case 3:
	//	m_floor->Render(m_direct3D, m_simpleShader, m_camera);
	//	break;

	//default:
	//	break;
	//}

	result = m_direct3D->EndScene();
	ASSERT(result);

	return true;
}