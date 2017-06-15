#pragma once

#include <d3d12.h>
#include <directxmath.h>
#include "../Direct3DManager.h"
#include "../../DirectX.h"
#include "../../d3dx12.h"
#include "MyMaterial.h"
#include "../Camera.h"

#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing fla
#include <vector>
#include <string>
#include <Windows.h>

using namespace DirectX;

class SimpleShader;

struct MyMaterial;
struct MyMesh;

class Model
{
public:
	Model();
	virtual ~Model();

	void RenderBuffers();
	bool Initialize(Direct3DManager* direct, std::string sFileName);
	void Render(Direct3DManager* direct, SimpleShader* shader, Camera* camera);

	void SetScale(XMFLOAT3& scale) { m_scale = XMFLOAT3(scale); }
	void SetRotation(XMFLOAT3& rotation) { m_rotation = XMFLOAT3(rotation); }
	void SetPosition(XMFLOAT3& position) { m_position = XMFLOAT3(position); }

	int GetIndexCount() const { return m_indices; }

	XMFLOAT3 GetPosition() const { return m_position; }
	XMFLOAT3 GetRotation() const { return m_rotation; }
	XMFLOAT3 GetScale() const { return m_scale; }
	XMMATRIX GetWorldMatrix() const { return m_worldMatrix; }

	std::vector<MyMaterial*> m_materials;
	std::vector<MyMesh*> m_meshes;
	XMFLOAT3 m_position;
	XMFLOAT3 m_rotation;
	XMFLOAT3 m_scale;
	XMMATRIX m_worldMatrix;
private:
	int m_indices;



protected:
};

struct VertexType
{
	VertexType() {}
	VertexType(float x, float y, float z, float u, float v) : position(x, y, z), texCoord(u, v) {}
	XMFLOAT3 position;
	XMFLOAT3 normal;
	XMFLOAT3 binormal;
	XMFLOAT3 tangent;
	XMFLOAT2 texCoord;
};
struct MyMaterial
{
	MyMaterial()
	{
		m_diffuseTexture = 0;
		m_normalTexture = 0;
		m_specularTexture = 0;
	}
	~MyMaterial()
	{
		SAFE_DELETE(m_diffuseTexture);
		SAFE_DELETE(m_normalTexture);
		SAFE_DELETE(m_specularTexture);
	}

	XMFLOAT3 m_Ka;
	XMFLOAT3 m_Kd;
	XMFLOAT3 m_Ks;

	Texture* m_diffuseTexture;
	Texture* m_normalTexture;
	Texture* m_specularTexture;
};
struct MyMesh
{
	MyMesh()
	{
		m_vertexBuffer = 0;
		m_indexBuffer = 0;
	}
	~MyMesh()
	{
		SAFE_RELEASE(m_vertexBuffer);
		SAFE_RELEASE(m_indexBuffer);
	}

	int m_materialIndex;

	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
	D3D12_INDEX_BUFFER_VIEW m_indexBufferView;
	ID3D12Resource* m_vertexBuffer; // a default buffer in GPU memory that we will load vertex data for our triangle into
	ID3D12Resource* m_indexBuffer; // a default buffer in GPU memory that we will load index data for our triangle into
};