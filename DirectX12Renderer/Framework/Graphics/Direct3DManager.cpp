#include "Direct3DManager.h"

Direct3DManager::Direct3DManager()
{
	m_device = 0;
	m_commandQueue = 0;
	m_swapChain = 0;
	m_renderTargetViewHeap = 0;
	m_commandList = 0;
	m_fenceEvent = 0;
	m_depthStencilViewHeap = 0;
	m_pipelineObject = 0;
	for (int i = 0; i < FRAME_BUFFER_COUNT; ++i)
	{
		m_backBufferRenderTarget[i] = 0;
		m_fence[i] = 0;
		m_commandAllocator[i] = 0;
	}
}

Direct3DManager::Direct3DManager(const Direct3DManager& other)
{

}

Direct3DManager::~Direct3DManager()
{
	if (m_swapChain)
		m_swapChain->SetFullscreenState(false, NULL);

	CloseHandle(m_fenceEvent);

	SAFE_RELEASE(m_device);
	SAFE_RELEASE(m_commandQueue);
	SAFE_RELEASE(m_swapChain);
	SAFE_RELEASE(m_renderTargetViewHeap);
	SAFE_RELEASE(m_commandList);
	SAFE_RELEASE(m_depthStencilViewHeap);
	for (int i = 0; i < FRAME_BUFFER_COUNT; ++i)
	{
		SAFE_RELEASE(m_commandAllocator[i]);
		SAFE_RELEASE(m_fence[i]);
		SAFE_RELEASE(m_backBufferRenderTarget[i]);
	}
}
bool Direct3DManager::Initialize(int screenHeight, int screenWidth, HWND hwnd, bool vsync, bool fullscreen, float screenDepth, float screenNear)
{
	HRESULT result;
	D3D_FEATURE_LEVEL featureLevel;
	IDXGIFactory4* factory;
	
	//Store the vsync setting
	m_vsync_enabled = vsync;

	//Set the feature level to directX 12.1 to enable using all the DirectX 12 features
	//Note: Not all cards support full Direct 12, this feature level may need to be reduced on some cards to 12.0
	featureLevel = D3D_FEATURE_LEVEL_12_0;

	//Create a DirectX graphics interface factory
	result = CreateDXGIFactory1(IID_PPV_ARGS(&factory));
	FAIL(result);

	IDXGIAdapter1* adapter1; // adapters are the graphics card (this includes the embedded graphics on the motherboard)
	int adapterIndex = 0; // we'll start looking for directx 12  compatible graphics devices starting at index 0

	bool adapterFound = false; // set this to true when a good one was found
	// find first hardware gpu that supports d3d 12
	while (factory->EnumAdapters1(adapterIndex, &adapter1) != DXGI_ERROR_NOT_FOUND)
	{
		DXGI_ADAPTER_DESC1 desc;
		adapter1->GetDesc1(&desc);

		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
		{
			// we dont want a software device
			continue;
		}

		// we want a device that is compatible with direct3d 12 (feature level 11 or higher)
		result = D3D12CreateDevice(adapter1, featureLevel, _uuidof(ID3D12Device), nullptr);
		if (SUCCEEDED(result))
		{
			adapterFound = true;
			break;
		}

		adapterIndex++;
	}

	//Create the Direct3D  12 device
	result = D3D12CreateDevice(adapter1, featureLevel, IID_PPV_ARGS(&m_device));
	if (FAILED(result))
	{
		MessageBox(hwnd, L"Could not create a DirectX 12.1 device. The default video card does not support DirectX 12.1.", L"DirectX Device Failure", MB_OK);
		return false;
	}

	D3D12_COMMAND_QUEUE_DESC cqDesc = {};
	cqDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	cqDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT; // direct means the gpu can directly execute this command queue

	//Create the command queue
	result = m_device->CreateCommandQueue(&cqDesc, __uuidof(ID3D12CommandQueue), (void**)&m_commandQueue);
	FAIL(result);

	// -- Create the Swap Chain (double/tripple buffering) -- //

	DXGI_MODE_DESC backBufferDesc = {}; // this is to describe our display mode
	backBufferDesc.Width = screenWidth; // buffer width
	backBufferDesc.Height = screenHeight; // buffer height
	backBufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // format of the buffer (rgba 32 bits, 8 bits for each chanel)

	// describe our multi-sampling. We are not multi-sampling, so we set the count to 1 (we need at least one sample of course)
	DXGI_SAMPLE_DESC sampleDesc = {};
	sampleDesc.Count = 1; // multisample count (no multisampling, so we just put 1, since we still need 1 sample)

	// Describe and create the swap chain.
	DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
	swapChainDesc.BufferCount = FRAME_BUFFER_COUNT; // number of buffers we have
	swapChainDesc.BufferDesc = backBufferDesc; // our back buffer description
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // this says the pipeline will render to this swap chain
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // dxgi will discard the buffer (data) after we call present
	swapChainDesc.OutputWindow = hwnd; // handle to our window
	swapChainDesc.SampleDesc = sampleDesc; // our multi-sampling description
	swapChainDesc.Windowed = !fullscreen; // set to true, then if in fullscreen must call SetFullScreenState with true for full screen to get uncapped fps

	IDXGISwapChain* tempSwapChain;

	result = factory->CreateSwapChain(m_commandQueue, &swapChainDesc, &tempSwapChain);
	FAIL(result);
	//Next upgrade the IDXGISwapChain to a IDXGISwapChain3 interface and store it in a private member variable named m_swapChain
	//this will allow us to ise the newer functionality such as getting the current back buffer index
	result = tempSwapChain->QueryInterface(__uuidof(IDXGISwapChain3), (void**)&m_swapChain);
	FAIL(result);


	//Release the factory now that the swap chain has been created
	SAFE_RELEASE(factory);

	//Finaly get the initial index to which buffer is the current back buffer
	m_bufferIndex = m_swapChain->GetCurrentBackBufferIndex();

	//Initialize the render target view heap description for the two back buffers
	D3D12_DESCRIPTOR_HEAP_DESC renderTargetViewHeapDesc = {};

	//Set the number of descriptors to two for our two back buffers. Also set the heap type to render target views
	renderTargetViewHeapDesc.NumDescriptors = FRAME_BUFFER_COUNT;
	renderTargetViewHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	renderTargetViewHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	//Craete the render target view heap for the back buffers
	result = m_device->CreateDescriptorHeap(&renderTargetViewHeapDesc, IID_PPV_ARGS(&m_renderTargetViewHeap));
	FAIL(result);

	//Get the size of the memory location for the render target view descriptors
	m_renderTargetViewDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	
	//Get a handle to the starting memory location in the render target view heap to identify where the render target views will be located for the two back buffers
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_renderTargetViewHeap->GetCPUDescriptorHandleForHeapStart());

	for (int i = 0; i < FRAME_BUFFER_COUNT; ++i)
	{
		//Get a pointer to the first back buffer from the swap chain
		result = m_swapChain->GetBuffer(i, IID_PPV_ARGS(&m_backBufferRenderTarget[i]));
		FAIL(result);

		//Create a render target view for the first back buffer
		m_device->CreateRenderTargetView(m_backBufferRenderTarget[i], nullptr, rtvHandle);

		//Increment the view handle to the next descriptor location in the render target view heap
		rtvHandle.Offset(1, m_renderTargetViewDescriptorSize);
	}


	// -- Create the Command Allocators -- //
	for (int i = 0; i < FRAME_BUFFER_COUNT; i++)
	{
		result = m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator[i]));
		FAIL(result);
	}

	//Create a basic command list
	result = m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator[m_bufferIndex], NULL, IID_PPV_ARGS(&m_commandList));
	FAIL(result);

	//Create a fence for GPU synchronization
	for (int i = 0; i < FRAME_BUFFER_COUNT; ++i)
	{
		result = m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence[i]));
		FAIL(result);
		m_fenceValue[i] = 0;
	}

	//Create an event object for the fence
	m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	FAIL(result);

	//CREATE stencils

	// Create depth stencil
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	result = m_device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_depthStencilViewHeap));
	FAIL(result);

	D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
	depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthStencilDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	depthStencilDesc.Flags = D3D12_DSV_FLAG_NONE;

	D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
	depthOptimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
	depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
	depthOptimizedClearValue.DepthStencil.Stencil = 0;

	result = m_device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, screenWidth, screenHeight, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL),
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&depthOptimizedClearValue,
		IID_PPV_ARGS(&m_depthStencilBuffer)
		);
	FAIL(result);
	m_depthStencilViewHeap->SetName(L"Depth/Stencil Resource Heap");

	m_device->CreateDepthStencilView(m_depthStencilBuffer, &depthStencilDesc, m_depthStencilViewHeap->GetCPUDescriptorHandleForHeapStart());


	// Setup the viewport for rendering.
	m_viewport.Width = (float)screenWidth;
	m_viewport.Height = (float)screenHeight;
	m_viewport.MinDepth = 0.0f;
	m_viewport.MaxDepth = 1.0f;
	m_viewport.TopLeftX = 0.0f;
	m_viewport.TopLeftY = 0.0f;

	m_commandList->RSSetViewports(1, &m_viewport);

	// Fill out a scissor rect
	m_scissorRect.left = 0;
	m_scissorRect.top = 0;
	m_scissorRect.right = screenWidth;
	m_scissorRect.bottom = screenHeight;

	m_commandList->RSSetScissorRects(1, &m_scissorRect);

	m_fieldOfView = 3.141592654f / 4.0f;
	m_screenAspect = (float)screenWidth / (float)screenHeight;

	// Create the projection matrix for 3D rendering.
	m_projectionMatrix = XMMatrixPerspectiveFovLH(m_fieldOfView, m_screenAspect, screenNear, screenDepth);
	
	// Initialize the world matrix to the identity matrix.
	m_worldMatrix = XMMatrixIdentity();

	//Initialize the starting fence value
	return true;
}
bool Direct3DManager::OpenCommandList()
{
	FAIL(m_commandList->Reset(m_commandAllocator[m_bufferIndex], m_pipelineObject));
	m_commandList->RSSetViewports(1, &m_viewport);
	m_commandList->RSSetScissorRects(1, &m_scissorRect);
	
	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_backBufferRenderTarget[m_bufferIndex], D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_renderTargetViewHeap->GetCPUDescriptorHandleForHeapStart(), m_bufferIndex, m_renderTargetViewDescriptorSize);
	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(m_depthStencilViewHeap->GetCPUDescriptorHandleForHeapStart());
	
	m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);
	return true;
}
bool Direct3DManager::ResetAllocator()
{
	WaitForPreviousFrame();
	
	FAIL(m_commandAllocator[m_bufferIndex]->Reset());

	return true;
}
bool Direct3DManager::BeginScene(float red, float green, float blue, float alpha)
{
	WaitForPreviousFrame();

	HRESULT result;

	//Reset (re-use) the memory associated command allocator
	result = m_commandAllocator[m_bufferIndex]->Reset();
	FAIL(result);

	//Reset the command list, use empty pipeline state for now since there are no shaders and we are just clearing the screen
	FAIL(m_commandList->Reset(m_commandAllocator[m_bufferIndex], m_pipelineObject));

	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_backBufferRenderTarget[m_bufferIndex], D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));
	
	// set the render target for the output merger stage (the output of the pipeline)
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_renderTargetViewHeap->GetCPUDescriptorHandleForHeapStart(), m_bufferIndex, m_renderTargetViewDescriptorSize);
	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(m_depthStencilViewHeap->GetCPUDescriptorHandleForHeapStart());
	// Clear RTV & DSV

	float color[4];

	// Setup the color to clear the buffer to.
	color[0] = red;
	color[1] = green;
	color[2] = blue;
	color[3] = alpha;

	m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);
	m_commandList->ClearRenderTargetView(rtvHandle, color, 0, nullptr);
	m_commandList->ClearDepthStencilView(m_depthStencilViewHeap->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	
	m_commandList->RSSetViewports(1, &m_viewport);
	m_commandList->RSSetScissorRects(1, &m_scissorRect);

	return true;
}

void Direct3DManager::ExecuteCommands()
{
	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_backBufferRenderTarget[m_bufferIndex], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));
	m_commandList->Close();
	ID3D12CommandList* ppCommandLists[] = { m_commandList };
	m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

}
bool Direct3DManager::EndScene()
{
	HRESULT result;
	unsigned long long fenceToWaitFor;

	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_backBufferRenderTarget[m_bufferIndex], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));
	m_commandList->Close();
	ID3D12CommandList* ppCommandLists[] = { m_commandList };
	m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
	
	FAIL(SignalFence());

	if (m_vsync_enabled)
	{
		//lock to screen refresh rate
		result = m_swapChain->Present(1, 0);
		FAIL(result);
	}
	else
	{
		//Present as fast as possible
		result = m_swapChain->Present(0, 0);
		FAIL(result);
	}

	return true;
}

bool Direct3DManager::WaitForPreviousFrame()
{
	HRESULT result;

	// swap the current rtv buffer index so we draw on the correct buffer
	m_bufferIndex = m_swapChain->GetCurrentBackBufferIndex();

	// if the current fence value is still less than "fenceValue", then we know the GPU has not finished executing
	// the command queue since it has not reached the "commandQueue->Signal(fence, fenceValue)" command
	if (m_fence[m_bufferIndex]->GetCompletedValue() < m_fenceValue[m_bufferIndex])
	{
		// we have the fence create an event which is signaled once the fence's current value is "fenceValue"
		result = m_fence[m_bufferIndex]->SetEventOnCompletion(m_fenceValue[m_bufferIndex], m_fenceEvent);
		FAIL(result);

		// We will wait until the fence has triggered the event that it's current value has reached "fenceValue". once it's value
		// has reached "fenceValue", we know the command queue has finished executing
		WaitForSingleObject(m_fenceEvent, INFINITE);
	}

	// increment fenceValue for next frame
	m_fenceValue[m_bufferIndex]++;

	return true;
}
HRESULT Direct3DManager::SignalFence()
{
	m_fenceValue[m_bufferIndex]++;
	return m_commandQueue->Signal(GetCurrentFence(), GetCurrentFenceValue());
}