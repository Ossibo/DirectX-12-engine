#pragma once

#include <d3d12.h>
#include <directxmath.h>
#include "../Direct3DManager.h"
#include "../../DirectX.h"
#include "../../d3dx12.h"
#include <Wincodec.h>
#include <objbase.h>
#include <d3d12.h>

class Texture
{
public:
	Texture();
	~Texture();
	bool Initialize(Direct3DManager* direct, LPCWSTR filenameDiffuse, int bufferIndex);
	void UploadTexture(Direct3DManager* direct);

	bool GetActive() { return m_active; }
	void SetActive(bool value){ m_active = value; }

private:
	int LoadImageDataFromFile(BYTE** imageData, D3D12_RESOURCE_DESC& resourceDescription, LPCWSTR filename, int &bytesPerRow);
	DXGI_FORMAT GetDXGIFormatFromWICFormat(WICPixelFormatGUID& wicFormatGUID);
	WICPixelFormatGUID GetConvertToWICFormat(WICPixelFormatGUID& wicFormatGUID);
	int GetDXGIFormatBitsPerPixel(DXGI_FORMAT& dxgiFormat);

private:

	int m_textureWidth;
	int m_textureHeight;

	int m_bufferIndex;

	bool m_active;

	ID3D12Resource* m_diffuseBuffer;
	ID3D12DescriptorHeap* m_diffuseDescriptorHeap;
	ID3D12Resource* m_diffuseBufferUploadHeap;
};