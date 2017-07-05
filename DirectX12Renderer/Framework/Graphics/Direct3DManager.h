#ifndef _DIRECT3DMANAGER_H_
#define _DIRECT3DMANAGER_H_

/////////////
// LINKING //
/////////////
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

//////////////
// INCLUDES //
//////////////
#include <d3d12.h>
#include <d3dcompiler.h>
#include <dxgi1_4.h>
#include <DirectXMath.h>

///////////////////////
// MY CLASS INCLUDES //
///////////////////////
#include "../DirectX.h"
#include "../d3dx12.h"

#define FRAME_BUFFER_COUNT 2

using namespace DirectX;

class Direct3DManager
{
public:
	Direct3DManager();
	Direct3DManager(const Direct3DManager& other);
	~Direct3DManager();

	bool Initialize(int screenHeight, int screenWidth, HWND hwnd, bool vsync, bool fullscreen, float screenDepth, float screenNear);
	ID3D12Device* GetDevice() { return m_device; }
	ID3D12GraphicsCommandList* GetCommandList() { return m_commandList; }
	ID3D12CommandQueue* GetCommandQueue() { return m_commandQueue; }
	ID3D12Fence* GetCurrentFence() { return m_fence[m_bufferIndex]; }
	unsigned long long GetCurrentFenceValue() { return m_fenceValue[m_bufferIndex]; }
	void IncrementFence() { m_fenceValue[m_bufferIndex]++; }
	bool OpenCommandList();
	bool ResetAllocator();
	void CreateNewCommandList(ID3D12GraphicsCommandList* list);
	HRESULT SignalFence();
	//bool Render();

	XMMATRIX GetWorldMatrix(){ return m_worldMatrix; }
	XMMATRIX GetProjectionMatrix() { return m_projectionMatrix; }

	bool BeginScene(float red, float green, float blue, float alpha);
	void ExecuteCommands();
	bool EndScene();

	unsigned int GetBufferIndex() { return m_bufferIndex; }


	int m_renderTargetViewDescriptorSize;
	unsigned int m_bufferIndex;
	ID3D12CommandQueue* m_commandQueue;
	ID3D12PipelineState* m_pipelineObject;
	ID3D12DescriptorHeap* m_renderTargetViewHeap;
	ID3D12DescriptorHeap* m_depthStencilViewHeap;
	ID3D12Resource* m_backBufferRenderTarget[FRAME_BUFFER_COUNT];
	ID3D12CommandAllocator* m_commandAllocator[FRAME_BUFFER_COUNT];
	D3D12_VIEWPORT m_viewport;
	D3D12_RECT m_scissorRect;



//private:
public:
	bool WaitForPreviousFrame();
//private:
public:
	bool m_vsync_enabled;
	ID3D12Device* m_device;
	char m_videoCardDescription[128];
	int m_videoCardMemory;
	IDXGISwapChain3* m_swapChain;
	ID3D12DescriptorHeap* m_mainDescriptorHeap;
	ID3D12Resource* m_depthStencilBuffer;
	ID3D12GraphicsCommandList* m_commandList;
	ID3D12RootSignature* m_rootSignature;
	ID3D12Fence* m_fence[FRAME_BUFFER_COUNT];
	unsigned long long m_fenceValue[FRAME_BUFFER_COUNT];
	HANDLE m_fenceEvent;

	XMMATRIX m_projectionMatrix;
	XMMATRIX m_worldMatrix;

	float m_fieldOfView;
	float m_screenAspect;
};

#endif