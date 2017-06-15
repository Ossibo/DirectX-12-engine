#pragma once
#include <d3dcompiler.h>
#include <directxmath.h>
#include "../Direct3DManager.h"
#include "../Objects/Model.h"
#include "../Camera.h"
//#include "../../d3dx12.h"

using namespace DirectX;

class SimpleShader
{
private:
	struct MatrixBufferType
	{
		XMMATRIX worldMatrix;
		XMMATRIX viewMatrix;
		XMMATRIX projectionMatrix;
	};
	struct LightBufferType
	{
		XMFLOAT4 lightColor;
		XMFLOAT3 lightPosition;
		float luminosity;
		XMFLOAT3 lightDirection;
		float padding;
		XMFLOAT3 cameraPos;
		float falloff;
	};
	struct PhongBufferType
	{
		XMFLOAT3 Ka;
		float padding1;
		XMFLOAT3 Kd;
		float padding2;
		XMFLOAT3 Ks;
		float specularPower;
	};
public:
	SimpleShader();
	~SimpleShader();
	bool Initialize(Direct3DManager* direct, HWND hwnd);
	bool Render(Direct3DManager* direct, Model* model, Camera* camera);
	void SetRootSignature(Direct3DManager* direct);
	void ChangeWireFrame() { m_wireFrame = !m_wireFrame; }
	ID3D12PipelineState* GetPipeline() { return m_pipelineState; }




	ID3D12RootSignature* m_rootSignature;
	ID3D12PipelineState* m_pipelineState;
private:
	bool InitializeShader(Direct3DManager* direct, HWND hwnd, WCHAR* vsFilename, WCHAR* psFilename);
	void OutputShaderErrorMsg(ID3D10Blob* errorMsg, HWND hwnd, WCHAR* shaderFilename);

	bool SetShaderParameters(ID3D12Device* device, XMMATRIX worldMatrix, XMMATRIX viewMatrix, XMMATRIX projectionMatrix);
	void RenderShader(ID3D12Device* device, int vertexCount, int instanceCount, int indexCount);

	bool m_wireFrame;

	ID3D12PipelineState* m_pipelineStateWithWireframe;
	
	UINT8* m_matrixBufferAdress[FRAME_BUFFER_COUNT];
	ID3D12Resource* m_matrixBuffer[FRAME_BUFFER_COUNT];
	MatrixBufferType m_matrixBufferType;

	UINT8* m_lightBufferAdress[FRAME_BUFFER_COUNT];
	ID3D12Resource* m_lightBuffer[FRAME_BUFFER_COUNT];
	LightBufferType m_lightBufferType;
	
	UINT8* m_phongBufferAdress[FRAME_BUFFER_COUNT];
	ID3D12Resource* m_phongBuffer[FRAME_BUFFER_COUNT];
	PhongBufferType m_phongBufferType;
};

