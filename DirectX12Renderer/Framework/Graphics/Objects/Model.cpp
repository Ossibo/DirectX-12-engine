#include "Model.h"
#include "../WIC/WICTextureLoader.h"
#include "../WIC/ResourceUploadBatch.h"
#include "../WIC/DirectXHelpers.h"
#include "../WIC/DescriptorHeap.h"
#include <wrl\client.h>
#include "../Shaders/SimpleShader.h"

using Microsoft::WRL::ComPtr;
Model::Model()
{
	m_position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_rotation = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_scale = XMFLOAT3(1.0f, 1.0f, 1.0f);
}

Model::~Model()
{
}
std::string GetDirectoryPath(std::string sFilePath)
{
	// Get directory path
	std::string sDirectory = "";
	for (int i = sFilePath.size() - 1; i >= 0; --i)
	{
		if (sFilePath[i] == '\\' || sFilePath[i] == '/')
		{
			sDirectory = sFilePath.substr(0, i + 1);
			break;
		}
	}
	return sDirectory;
}
std::wstring s2ws(const std::string& str)
{
	int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
	std::wstring wstrTo(size_needed, 0);
	MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
	return wstrTo;
}
bool Model::Initialize(Direct3DManager* direct, std::string sFilePath)
{
	ID3D12Device* device = direct->GetDevice();
	ID3D12GraphicsCommandList* commandList = direct->GetCommandList();
	HRESULT result;

	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(sFilePath,
											aiProcess_CalcTangentSpace |
											aiProcess_GenSmoothNormals |
											aiProcess_ImproveCacheLocality |
											aiProcess_Triangulate |
											aiProcess_JoinIdenticalVertices |
											aiProcess_OptimizeMeshes |
											aiProcess_OptimizeGraph |
											aiProcess_LimitBoneWeights |
											aiProcess_RemoveRedundantMaterials |
											aiProcess_SplitLargeMeshes |
											aiProcess_GenUVCoords |
											aiProcess_FindDegenerates |
											aiProcess_FindInvalidData |
											aiProcess_MakeLeftHanded |
											aiProcess_FlipUVs |
											aiProcess_FlipWindingOrder |
											0);
	for (int i = 0; i < scene->mNumMaterials; ++i)
	{
		MyMaterial* newMaterial = new MyMaterial();
		const aiMaterial* material = scene->mMaterials[i];
		
		aiColor3D color;
		if (material->Get(AI_MATKEY_COLOR_AMBIENT, color) == AI_SUCCESS);
		{
			newMaterial->m_Ka = XMFLOAT3(color.r, color.g, color.b);
		}
		if (material->Get(AI_MATKEY_COLOR_DIFFUSE, color) == AI_SUCCESS);
		{
			newMaterial->m_Kd = XMFLOAT3(color.r, color.g, color.b);
		}
		if (material->Get(AI_MATKEY_COLOR_SPECULAR, color) == AI_SUCCESS);
		{
			newMaterial->m_Ks = XMFLOAT3(color.r, color.g, color.b);
		}
		
		aiString path;  // filename
		if ((material->GetTexture(aiTextureType_DIFFUSE, 0, &path) == AI_SUCCESS))
		{
			newMaterial->m_diffuseTexture = new Texture();
			std::string sDir = GetDirectoryPath(sFilePath);
			std::string fileName = path.data;
			std::string sFullPathDiffuse = sDir + fileName;
			ASSERT(newMaterial->m_diffuseTexture->Initialize(direct, s2ws(sFullPathDiffuse).c_str(), 3));
			newMaterial->m_diffuseTexture->SetActive(true);
		}
		if ((material->GetTexture(aiTextureType_HEIGHT, 0, &path) == AI_SUCCESS))
		{
			newMaterial->m_normalTexture = new Texture();
			std::string sDir = GetDirectoryPath(sFilePath);
			std::string fileName = path.data;
			std::string sFullPathNormal = sDir + fileName;
			ASSERT(newMaterial->m_normalTexture->Initialize(direct, s2ws(sFullPathNormal).c_str(), 4));
			newMaterial->m_normalTexture->SetActive(true);
		}
		if ((material->GetTexture(aiTextureType_SPECULAR, 0, &path) == AI_SUCCESS))
		{
			newMaterial->m_specularTexture = new Texture();
			std::string sDir = GetDirectoryPath(sFilePath);
			std::string fileName = path.data;
			std::string sFullPathSpecular = sDir + fileName;
			ASSERT(newMaterial->m_specularTexture->Initialize(direct, s2ws(sFullPathSpecular).c_str(), 5));
			newMaterial->m_specularTexture->SetActive(true);
		}
		m_materials.push_back(newMaterial);
		newMaterial = 0;
	}

	for (int i = 0; i < scene->mNumMeshes; ++i)
	{
		MyMesh* newMesh = new MyMesh();

		
		aiMesh* ourMesh = scene->mMeshes[i];
		aiString name = ourMesh->mName;
		aiVector3D* myVertices = ourMesh->mVertices;
		aiVector3D* myNormals = ourMesh->mNormals;
		aiVector3D* myBinormals = ourMesh->mBitangents;
		aiVector3D* myTangents = ourMesh->mTangents;
		aiVector3D* myTexCoords = ourMesh->mTextureCoords[0];
		int totalVertices = ourMesh->mNumVertices;
		std::vector<XMFLOAT3> test;

		for (int j = 0; j < totalVertices; ++j)
		{
			test.push_back(XMFLOAT3(myVertices[j].x, myVertices[j].y, myVertices[j].z));
		}
		std::string name1 = name.C_Str();
		newMesh->m_materialIndex = ourMesh->mMaterialIndex;

		VertexType* vertices = new VertexType[totalVertices];
		for (int j = 0; j < totalVertices; ++j)
		{
			vertices[j].position = XMFLOAT3(myVertices[j].x, myVertices[j].y, myVertices[j].z);

			if (myNormals)
				vertices[j].normal = XMFLOAT3(myNormals[j].x, myNormals[j].y, myNormals[j].z);
			if (myBinormals)
				vertices[j].binormal = XMFLOAT3(myBinormals[j].x, myBinormals[j].y, myBinormals[j].z);
			if (myTangents)
				vertices[j].tangent = XMFLOAT3(myTangents[j].x, myTangents[j].y, myTangents[j].z);
			if (myTexCoords)
				vertices[j].texCoord = XMFLOAT2(myTexCoords[j].x, myTexCoords[j].y);

		}

		int vBufferSize = sizeof(VertexType) * totalVertices;

		result = device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), // a default heap
			D3D12_HEAP_FLAG_NONE, // no flags
			&CD3DX12_RESOURCE_DESC::Buffer(vBufferSize), // resource description for a buffer
			D3D12_RESOURCE_STATE_COPY_DEST, // we will start this heap in the copy destination state since we will copy data
			// from the upload heap to this heap
			nullptr, // optimized clear value must be null for this type of resource. used for render targets and depth/stencil buffers
			IID_PPV_ARGS(&newMesh->m_vertexBuffer));
		FAIL(result);

		newMesh->m_vertexBuffer->SetName(L"Vertex Buffer Resource Heap"); // Debug purposes

		ID3D12Resource* vBufferUploadHeap;
		result = device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), // upload heap
			D3D12_HEAP_FLAG_NONE, // no flags
			&CD3DX12_RESOURCE_DESC::Buffer(vBufferSize), // resource description for a buffer
			D3D12_RESOURCE_STATE_GENERIC_READ, // GPU will read from this buffer and copy its contents to the default heap
			nullptr,
			IID_PPV_ARGS(&vBufferUploadHeap));
		FAIL(result);

		vBufferUploadHeap->SetName(L"Vertex Buffer Upload Resource Heap");

		D3D12_SUBRESOURCE_DATA vertexData = {};
		vertexData.pData = reinterpret_cast<BYTE*>(vertices); // pointer to our vertex array
		vertexData.RowPitch = vBufferSize; // size of all our triangle vertex data
		vertexData.SlicePitch = vBufferSize; // also the size of our triangle vertex data

		UpdateSubresources(commandList, newMesh->m_vertexBuffer, vBufferUploadHeap, 0, 0, 1, &vertexData);

		// transition the vertex buffer data from copy destination state to vertex buffer state
		commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(newMesh->m_vertexBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));

		int totalIndices = 0;
		for (int j = 0; j < ourMesh->mNumFaces; ++j)
		{
			totalIndices += ourMesh->mFaces[j].mNumIndices;
		}

		DWORD* indices = new DWORD[totalIndices];
		int indexCount = 0;
		for (int j = 0; j < ourMesh->mNumFaces; ++j)
		{
			aiFace face = ourMesh->mFaces[j];
			for (int k = 0; k < face.mNumIndices; ++k)
			{
				indices[indexCount] = face.mIndices[k];
				++indexCount;
			}
		}

		int iBufferSize = sizeof(DWORD) * totalIndices;

		m_indices = totalIndices;

		result = device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), // a default heap
			D3D12_HEAP_FLAG_NONE, // no flags
			&CD3DX12_RESOURCE_DESC::Buffer(iBufferSize), // resource description for a buffer
			D3D12_RESOURCE_STATE_COPY_DEST, // start in the copy destination state
			nullptr, // optimized clear value must be null for this type of resource
			IID_PPV_ARGS(&newMesh->m_indexBuffer));
		FAIL(result);

		newMesh->m_vertexBuffer->SetName(L"Index Buffer Resource Heap");

		// create upload heap to upload index buffer
		ID3D12Resource* iBufferUploadHeap;
		result = device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), // upload heap
			D3D12_HEAP_FLAG_NONE, // no flags
			&CD3DX12_RESOURCE_DESC::Buffer(vBufferSize), // resource description for a buffer
			D3D12_RESOURCE_STATE_GENERIC_READ, // GPU will read from this buffer and copy its contents to the default heap
			nullptr,
			IID_PPV_ARGS(&iBufferUploadHeap));
		FAIL(result);

		vBufferUploadHeap->SetName(L"Index Buffer Upload Resource Heap");

		D3D12_SUBRESOURCE_DATA indexData = {};
		indexData.pData = reinterpret_cast<BYTE*>(indices); // pointer to our index array
		indexData.RowPitch = iBufferSize; // size of all our index buffer
		indexData.SlicePitch = iBufferSize; // also the size of our index buffer

		UpdateSubresources(commandList, newMesh->m_indexBuffer, iBufferUploadHeap, 0, 0, 1, &indexData);

		commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(newMesh->m_indexBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));

		// Vertex Buffer View
		newMesh->m_vertexBufferView.BufferLocation = newMesh->m_vertexBuffer->GetGPUVirtualAddress();
		newMesh->m_vertexBufferView.StrideInBytes = sizeof(VertexType);
		newMesh->m_vertexBufferView.SizeInBytes = vBufferSize;

		// Index Buffer View
		// create a vertex buffer view for the triangle. We get the GPU memory address to the vertex pointer using the GetGPUVirtualAddress() method
		newMesh->m_indexBufferView.BufferLocation = newMesh->m_indexBuffer->GetGPUVirtualAddress();
		newMesh->m_indexBufferView.Format = DXGI_FORMAT_R32_UINT; // 32-bit unsigned integer (this is what a dword is, double word, a word is 2 bytes)
		newMesh->m_indexBufferView.SizeInBytes = iBufferSize;

		m_meshes.push_back(newMesh);

		// Now we execute the command list to upload the initial assets (triangle data)


	}
	return true;
}

void Model::Render(Direct3DManager* direct, SimpleShader* shader, Camera* camera)
{
	ID3D12GraphicsCommandList* commandList = direct->GetCommandList();

	m_worldMatrix = XMMatrixScaling(m_scale.x, m_scale.y, m_scale.z) * XMMatrixRotationX(m_rotation.x) * XMMatrixRotationY(m_rotation.y) * XMMatrixRotationZ(m_rotation.z) * XMMatrixTranslation(m_position.x, m_position.y, m_position.z);
	direct->ResetAllocator();
	for (int i = 0; i < m_meshes.size(); ++i)
	{
		direct->OpenCommandList();
		shader->SetRootSignature(direct);

		MyMesh* mesh = m_meshes[i];
		MyMaterial* material = m_materials[mesh->m_materialIndex];

		if (material->m_diffuseTexture)
			material->m_diffuseTexture->UploadTexture(direct);

		if (material->m_normalTexture)
			material->m_normalTexture->UploadTexture(direct);

		if (material->m_specularTexture)
			material->m_specularTexture->UploadTexture(direct);
	
		commandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		commandList->IASetVertexBuffers(0, 1, &mesh->m_vertexBufferView);
		commandList->IASetIndexBuffer(&mesh->m_indexBufferView);
		shader->Render(direct, this, camera);
		direct->ExecuteCommands();
	}

}