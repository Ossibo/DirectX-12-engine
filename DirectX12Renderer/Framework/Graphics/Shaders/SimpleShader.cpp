#include "SimpleShader.h"


SimpleShader::SimpleShader()
{
	m_pipelineState = 0;
	m_rootSignature = 0;
	m_wireFrame = false;
}


SimpleShader::~SimpleShader()
{
	SAFE_RELEASE(m_pipelineState);
	SAFE_RELEASE(m_rootSignature);
}

bool SimpleShader::Initialize(Direct3DManager* direct, HWND hwnd)
{
	bool result;

	result = InitializeShader(direct, hwnd, L"Shaders/SimpleVS.hlsl", L"Shaders/SimplePS.hlsl");
	ASSERT(result);

	return true;
}

// Create shader pipeline
bool SimpleShader::InitializeShader(Direct3DManager* direct, HWND hwnd, WCHAR* vsFilename, WCHAR* psFilename)
{
	ID3D12Device* device = direct->GetDevice();

	HRESULT result;
	// describe our multi-smapling. we are not multi-sampling, so we set the count to 1 ( we need at least one sample of course)
	DXGI_SAMPLE_DESC sampleDesc = {};
	sampleDesc.Count = 1; // Multisample count (no multisampling, so we just put 1, since we still need 1 sample

	// Create root signature

	// Create a root descriptor, which explains where to find the data for this root parameter
	D3D12_ROOT_DESCRIPTOR rootCBVDescriptor;
	rootCBVDescriptor.RegisterSpace = 0;
	rootCBVDescriptor.ShaderRegister = 0;

	D3D12_ROOT_DESCRIPTOR rootCBVDescriptor2;
	rootCBVDescriptor2.RegisterSpace = 0;
	rootCBVDescriptor2.ShaderRegister = 0;

	D3D12_ROOT_DESCRIPTOR rootCBVDescriptor3;
	rootCBVDescriptor3.RegisterSpace = 0;
	rootCBVDescriptor3.ShaderRegister = 1;

	// Create a descriptor range (descritor table) and fill it our
	// This is a range of descriptors indside a descriptor heap
	D3D12_DESCRIPTOR_RANGE descriptorTableRanges[1]; // Only one range right now
	descriptorTableRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV; // This is a range of shader resource view (descriptors)
	descriptorTableRanges[0].NumDescriptors = 1; // We only have one texture right now, so the range is only 1
	descriptorTableRanges[0].BaseShaderRegister = 0; // Start index of the shader registers in the range
	descriptorTableRanges[0].RegisterSpace = 0; // Space 0. can usually be zero
	descriptorTableRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND; // this appends the range to the end of the root signature descriptor tables

	// Create a descriptor table
	D3D12_ROOT_DESCRIPTOR_TABLE descriptorTable;
	descriptorTable.NumDescriptorRanges = _countof(descriptorTableRanges); // We only have one range
	descriptorTable.pDescriptorRanges = &descriptorTableRanges[0]; // The pointer to the beginning of our ranges array

	// Create a descriptor range (descritor table) and fill it our
	// This is a range of descriptors indside a descriptor heap
	D3D12_DESCRIPTOR_RANGE descriptorTableRanges2[1]; // Only one range right now
	descriptorTableRanges2[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV; // This is a range of shader resource view (descriptors)
	descriptorTableRanges2[0].NumDescriptors = 1; // We only have one texture right now, so the range is only 1
	descriptorTableRanges2[0].BaseShaderRegister = 1; // Start index of the shader registers in the range
	descriptorTableRanges2[0].RegisterSpace = 0; // Space 0. can usually be zero
	descriptorTableRanges2[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND; // this appends the range to the end of the root signature descriptor tables

	// Create a descriptor table
	D3D12_ROOT_DESCRIPTOR_TABLE descriptorTable2;
	descriptorTable2.NumDescriptorRanges = _countof(descriptorTableRanges2); // We only have one range
	descriptorTable2.pDescriptorRanges = &descriptorTableRanges2[0]; // The pointer to the beginning of our ranges array

	// This is a range of descriptors indside a descriptor heap
	D3D12_DESCRIPTOR_RANGE descriptorTableRanges3[1]; // Only one range right now
	descriptorTableRanges3[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV; // This is a range of shader resource view (descriptors)
	descriptorTableRanges3[0].NumDescriptors = 1; // We only have one texture right now, so the range is only 1
	descriptorTableRanges3[0].BaseShaderRegister = 2; // Start index of the shader registers in the range
	descriptorTableRanges3[0].RegisterSpace = 0; // Space 0. can usually be zero
	descriptorTableRanges3[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND; // this appends the range to the end of the root signature descriptor tables

	// Create a descriptor table
	D3D12_ROOT_DESCRIPTOR_TABLE descriptorTable3;
	descriptorTable3.NumDescriptorRanges = _countof(descriptorTableRanges3); // We only have one range
	descriptorTable3.pDescriptorRanges = &descriptorTableRanges3[0]; // The pointer to the beginning of our ranges array
	int index = 0;
	// Create a root parameter for the root descriptor and fill it out
	D3D12_ROOT_PARAMETER rootParameters[6]; // Two root parameters
	rootParameters[index].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; // This is a constant buffer view root descriptor
	rootParameters[index].Descriptor = rootCBVDescriptor; // This is the root descriptor for this root parameter
	rootParameters[index].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX; // Our pixel shader will be the only shader accessing this parameter for now
	++index;
	rootParameters[index].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; // This is a constant buffer view root descriptor
	rootParameters[index].Descriptor = rootCBVDescriptor2; // This is the root descriptor for this root parameter
	rootParameters[index].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // Our pixel shader will be the only shader accessing this parameter for now
	++index;
	rootParameters[index].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; // This is a constant buffer view root descriptor
	rootParameters[index].Descriptor = rootCBVDescriptor3; // This is the root descriptor for this root parameter
	rootParameters[index].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // Our pixel shader will be the only shader accessing this parameter for now
	++index;
	// Fill out the parameter for our descriptor table. Remember it's a good idea to sort parameters by frequency of change. Our constant
	// buffer will be changed multiple times per frame, while our descritor table will not be changed at all for now
	rootParameters[index].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE; // This is a descriptor table
	rootParameters[index].DescriptorTable = descriptorTable; // This is our descriptor table for this root parameter
	rootParameters[index].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // Our pixel shader will be the only shader accessing this parameter for now
	++index;
	// Fill out the parameter for our descriptor table. Remember it's a good idea to sort parameters by frequency of change. Our constant
	// buffer will be changed multiple times per frame, while our descritor table will not be changed at all for now
	rootParameters[index].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE; // This is a descriptor table
	rootParameters[index].DescriptorTable = descriptorTable2; // This is our descriptor table for this root parameter
	rootParameters[index].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // Our pixel shader will be the only shader accessing this parameter for now
	++index;
	// Fill out the parameter for our descriptor table. Remember it's a good idea to sort parameters by frequency of change. Our constant
	// buffer will be changed multiple times per frame, while our descritor table will not be changed at all for now
	rootParameters[index].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE; // This is a descriptor table
	rootParameters[index].DescriptorTable = descriptorTable3; // This is our descriptor table for this root parameter
	rootParameters[index].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // Our pixel shader will be the only shader accessing this parameter for now
	++index;
	// Create a static sampler
	D3D12_STATIC_SAMPLER_DESC sampler = {};
	sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	sampler.MipLODBias = 0;
	sampler.MaxAnisotropy = 16;
	sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	sampler.MinLOD = 0.0f;
	sampler.MaxLOD = D3D12_FLOAT32_MAX;
	sampler.ShaderRegister = 0;
	sampler.RegisterSpace = 0;
	sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init(_countof(rootParameters), // we have 2 root parameters
		rootParameters, // a pointer to the beginning of our root parameters array
		1, // we have one static sampler
		&sampler, // a pointer to our static sampler (array)
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | // we can deny shader stages here for better performance
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS);

	ID3DBlob* errorBuff; // A buffer holding the error data if any
	ID3DBlob* signature;
	result = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &errorBuff);
	FAIL(result);

	result = device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature));
	FAIL(result);

	// Compile vertex shadern
	ID3DBlob* vertexShader;
	result = D3DCompileFromFile(vsFilename, nullptr, nullptr, "main", "vs_5_0", D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &vertexShader, &errorBuff);
	if (FAILED(result))
	{
		OutputDebugStringA((char*)errorBuff->GetBufferPointer());
		return false;
	}

	// Fill out shader byte structure, which is basically just a pointer
	// to the shader byteconde and the size of the shader bytecode
	D3D12_SHADER_BYTECODE vertexShaderBytecode = {};
	vertexShaderBytecode.BytecodeLength = vertexShader->GetBufferSize();
	vertexShaderBytecode.pShaderBytecode = vertexShader->GetBufferPointer();

	// Compile pixel shadern
	ID3DBlob* pixelShader;
	result = D3DCompileFromFile(psFilename, nullptr, nullptr, "main", "ps_5_0", D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &pixelShader, &errorBuff);
	if (FAILED(result))
	{
		OutputDebugStringA((char*)errorBuff->GetBufferPointer());
		return false;
	}

	// Fill out shader byteconde structure for pixel shader
	D3D12_SHADER_BYTECODE pixelShaderBytecode = {};
	pixelShaderBytecode.BytecodeLength = pixelShader->GetBufferSize();
	pixelShaderBytecode.pShaderBytecode = pixelShader->GetBufferPointer();

	// Create input layout

	// This input layout is used by the Input Assembler so that is knows how to read the vertex data bound to it
	D3D12_INPUT_ELEMENT_DESC inputLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 36, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 48, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	// Fill out an input layour description structure
	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc = {};

	// We can get the number of elements on an array by "sizeof(array) / sizeof(arrayElementType)"
	inputLayoutDesc.NumElements = sizeof(inputLayout) / sizeof(D3D12_INPUT_ELEMENT_DESC);
	inputLayoutDesc.pInputElementDescs = inputLayout;

	// Create a popeline state object(PSO)

	// In a real application you will have many pso's. for each different shader
	// or different combinations of shaders, different blend states of different rasterizer states,
	// differrent topology tpyes (point, line, triangle, patch), or a different number of render targets you will need a pso

	// VS is the only required shader for a pso. You might be wondering when a case would be where you only
	// set the VS. It's possible that you have a pso that only outputs data with the stream output, and
	// not on a render target, which means you would not need anything after the stream output.

	D3D12_RASTERIZER_DESC rasterizerDesc = {};
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	rasterizerDesc.FrontCounterClockwise = FALSE;
	rasterizerDesc.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
	rasterizerDesc.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
	rasterizerDesc.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
	rasterizerDesc.DepthClipEnable = TRUE;
	rasterizerDesc.MultisampleEnable = FALSE;
	rasterizerDesc.AntialiasedLineEnable = FALSE;
	rasterizerDesc.ForcedSampleCount = 0;
	rasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.InputLayout = inputLayoutDesc;
	psoDesc.pRootSignature = m_rootSignature;
	psoDesc.VS = vertexShaderBytecode;
	psoDesc.PS = pixelShaderBytecode;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.SampleDesc = sampleDesc;
	psoDesc.SampleMask = 0xffffffff;
	psoDesc.RasterizerState = rasterizerDesc;
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.NumRenderTargets = 1;
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);

	result = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState));

	rasterizerDesc.FillMode = D3D12_FILL_MODE_WIREFRAME;
	psoDesc.RasterizerState = rasterizerDesc;

	result = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineStateWithWireframe));
	FAIL(result);
	for (int i = 0; i < FRAME_BUFFER_COUNT; ++i)
	{
		result = device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), // this heap will be used to upload the constant buffer data
			D3D12_HEAP_FLAG_NONE, // no flags
			&CD3DX12_RESOURCE_DESC::Buffer(D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT), // size of the resource heap. Must be a multiple of 64KB for single-textures and constant buffers
			D3D12_RESOURCE_STATE_GENERIC_READ, // will be data that is read from so we keep it in the generic read state
			nullptr, // we do not have use an optimized clear value for constant buffers
			IID_PPV_ARGS(&m_matrixBuffer[i]));
		FAIL(result);

		m_matrixBuffer[i]->SetName(L"Constant Matrix Buffer Upload Resource Heap ");

		ZeroMemory(&m_matrixBufferType, sizeof(m_matrixBufferType));

		CD3DX12_RANGE readRange(0, 0);    // We do not intend to read from this resource on the CPU. (so end is less than or equal to begin)

		result = m_matrixBuffer[i]->Map(0, &readRange, reinterpret_cast<void**>(&m_matrixBufferAdress[i]));

		memcpy(m_matrixBufferAdress[i], &m_matrixBufferType, sizeof(m_matrixBufferType));
	}

	// LightBuffer
	for (int i = 0; i < FRAME_BUFFER_COUNT; ++i)
	{
		result = device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), // this heap will be used to upload the constant buffer data
			D3D12_HEAP_FLAG_NONE, // no flags
			&CD3DX12_RESOURCE_DESC::Buffer(D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT), // size of the resource heap. Must be a multiple of 64KB for single-textures and constant buffers
			D3D12_RESOURCE_STATE_GENERIC_READ, // will be data that is read from so we keep it in the generic read state
			nullptr, // we do not have use an optimized clear value for constant buffers
			IID_PPV_ARGS(&m_lightBuffer[i]));
		FAIL(result);

		m_lightBuffer[i]->SetName(L"Constant Matrix Buffer Upload Resource Heap ");

		ZeroMemory(&m_matrixBufferType, sizeof(m_matrixBufferType));

		CD3DX12_RANGE readRange(0, 0);    // We do not intend to read from this resource on the CPU. (so end is less than or equal to begin)

		result = m_lightBuffer[i]->Map(0, &readRange, reinterpret_cast<void**>(&m_lightBufferAdress[i]));

		memcpy(m_lightBufferAdress[i], &m_lightBufferType, sizeof(m_lightBufferType));
	}

	// PhongBuffer
	for (int i = 0; i < FRAME_BUFFER_COUNT; ++i)
	{
		result = device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), // this heap will be used to upload the constant buffer data
			D3D12_HEAP_FLAG_NONE, // no flags
			&CD3DX12_RESOURCE_DESC::Buffer(D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT), // size of the resource heap. Must be a multiple of 64KB for single-textures and constant buffers
			D3D12_RESOURCE_STATE_GENERIC_READ, // will be data that is read from so we keep it in the generic read state
			nullptr, // we do not have use an optimized clear value for constant buffers
			IID_PPV_ARGS(&m_phongBuffer[i]));
		FAIL(result);

		m_phongBuffer[i]->SetName(L"Constant Matrix Buffer Upload Resource Heap ");

		ZeroMemory(&m_phongBufferType, sizeof(m_phongBufferType));

		CD3DX12_RANGE readRange(0, 0);    // We do not intend to read from this resource on the CPU. (so end is less than or equal to begin)

		result = m_phongBuffer[i]->Map(0, &readRange, reinterpret_cast<void**>(&m_phongBufferAdress[i]));

		memcpy(m_phongBufferAdress[i], &m_phongBufferType, sizeof(m_phongBufferType));
	}

	return true;
}

void SimpleShader::SetRootSignature(Direct3DManager* direct)
{
	ID3D12GraphicsCommandList* commandList = direct->GetCommandList();
	
	if (m_wireFrame)
		commandList->SetPipelineState(m_pipelineStateWithWireframe);
	else
		commandList->SetPipelineState(m_pipelineState);
	
	commandList->SetGraphicsRootSignature(m_rootSignature);
}
bool SimpleShader::Render(Direct3DManager* direct, Model* model, Camera* camera)
{
	ID3D12GraphicsCommandList* commandList = direct->GetCommandList();
	unsigned int bufferIndex = direct->GetBufferIndex();

	m_matrixBufferType.worldMatrix = model->GetWorldMatrix();
	m_matrixBufferType.viewMatrix = camera->GetViewMatrix();
	m_matrixBufferType.projectionMatrix = direct->GetProjectionMatrix();


	memcpy(m_matrixBufferAdress[bufferIndex], &m_matrixBufferType, sizeof(m_matrixBufferType));
	commandList->SetGraphicsRootConstantBufferView(0, m_matrixBuffer[bufferIndex]->GetGPUVirtualAddress());

	m_lightBufferType.cameraPos = camera->GetPosition();
	m_lightBufferType.falloff = 200;
	m_lightBufferType.lightColor = XMFLOAT4(0.6f, 0.6f, 0.6f, 1.0f);
	m_lightBufferType.lightPosition = XMFLOAT3(-0.0f, 000.0f, -1000.0f);
	m_lightBufferType.lightDirection = XMFLOAT3(0.0f, -1.f, 1.0f);
	m_lightBufferType.luminosity = 1.0f;
	
	memcpy(m_lightBufferAdress[bufferIndex], &m_lightBufferType, sizeof(m_lightBufferType));
	commandList->SetGraphicsRootConstantBufferView(1, m_lightBuffer[bufferIndex]->GetGPUVirtualAddress());

	m_phongBufferType.Ka = XMFLOAT3(0.1f, 0.1f, 0.1f);
	m_phongBufferType.Kd = XMFLOAT3(0.7f, 0.7f, 0.7f);
	m_phongBufferType.Ks = XMFLOAT3(1.0f, 1.0f, 1.0f);
	m_phongBufferType.specularPower = 1.0f;

	memcpy(m_phongBufferAdress[bufferIndex], &m_phongBufferType, sizeof(m_phongBufferType));
	commandList->SetGraphicsRootConstantBufferView(2, m_phongBuffer[bufferIndex]->GetGPUVirtualAddress());
	commandList->DrawIndexedInstanced(model->GetIndexCount(), 1, 0, 0, 0);

	return true;
}