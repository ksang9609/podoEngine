#pragma once

#include <d3d11.h>
#include <wrl/client.h>
#include "Core//Core.h"

struct alignas(16) FHiZBufferConstants
{
	uint32 DstWidth;
	uint32 DstHeight;
	float InvDstWidth;
	float InvDstHeight;

	float SrcTexelSizeX;
	float SrcTexelSizeY;
	uint32 IsPass0;
	uint32 Pad;	
};

class FHiZBuffer
{
	template<typename T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;

public:
	static constexpr uint32 HZBWidth = 1024;
	static constexpr uint32 HZBHeight = 512;
	static constexpr uint32 MipLevels = 11; // 1024x512 to 1x1

	void Initialize(ID3D11Device* device);
	void Release();

	void BuildHiZ(ID3D11DeviceContext* context, ID3D11ShaderResourceView* mainDepthSRV, uint32 screenWidth, uint32 screenHeight);

	ID3D11ShaderResourceView* GetHzbSRV() const { return mHzbFullSRV.Get(); }

private:
	ComPtr<ID3D11Texture2D> mHzbTexture = nullptr; // Hi-Z buffer Texture
	ComPtr<ID3D11ShaderResourceView> mHzbFullSRV = nullptr; // All SRV for Mip 0~10

	ComPtr<ID3D11UnorderedAccessView> mHzbMipUAV[MipLevels]; // Read
	ComPtr<ID3D11ShaderResourceView> mHzbMipSRV[MipLevels]; // Write;

	ComPtr<ID3D11ComputeShader> mHzBuildCS = nullptr;
	ComPtr<ID3D11Buffer> mConstantBuffer = nullptr;
	ComPtr<ID3D11SamplerState> mPointClampSampler = nullptr;

	void createResources(ID3D11Device* device);
	void createShader(ID3D11Device* device);
};
