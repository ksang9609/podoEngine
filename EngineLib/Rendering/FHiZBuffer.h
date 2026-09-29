#pragma once

#include <d3d11.h>
#include <wrl/client.h>
#include "Core/Math/Vector.h"
#include "Core/Math/Matrix.h"
#include "Core/Core.h"
#include "RenderInfo.h"

struct alignas(16) FHiZBufferConstants
{
	uint32 DstWidth;
	uint32 DstHeight;
	float InvDstWidth;
	float InvDstHeight;

	float ViewportUVOffsetX;
	float ViewportUVOffsetY;
	float ViewportUVScaleX;
	float ViewportUVScaleY;
	
	uint32 IsPass0;
	uint32 Pad[3];	
};

struct alignas(16) FGpuAABB
{
	FVector Min;
	float Pad1;
	FVector Max;
	float Pad2;
};

struct alignas(16) FCullConstants
{
	FMatrix ViewProjection;
	float ScreenWidth;
	float ScreenHeight;
	uint32 NumObject;
	float NearPlane;
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

	void BuildHiZ(ID3D11DeviceContext* context, ID3D11ShaderResourceView* mainDepthSRV, float viewportX, float viewportY,
		float viewportWidth, float viewportHeight, float totalBufferWidth, float totalBufferHeight);
	void ExecuteOcclusionCull(ID3D11DeviceContext* context,
		ID3D11Device* device,
		const TArray<const FRenderInfo*>& renderInfos,
		const FMatrix& viewProjectionMatrix,
		float screenWidth,
		float screenHeight,
		float nearPlane
	);

	const uint32* ReadbackVisibility(ID3D11DeviceContext* contex, uint32& outCount);
	void UnmapVisibility(ID3D11DeviceContext* context);

	void ResetStagingState() { mHasValidStagingData = false; }

	ID3D11ShaderResourceView* GetHzbSRV() const { return mHzbFullSRV.Get(); }

private:
	ComPtr<ID3D11Texture2D> mHzbTexture = nullptr; // Hi-Z buffer Texture	

	ComPtr<ID3D11UnorderedAccessView> mHzbMipUAV[MipLevels]; // Read
	ComPtr<ID3D11ShaderResourceView> mHzbMipSRV[MipLevels]; // Write;
	ComPtr<ID3D11ShaderResourceView> mHzbFullSRV = nullptr; // All SRV for Mip 0~10	

	ComPtr<ID3D11ComputeShader> mHzBuildCS = nullptr;
	ComPtr<ID3D11ComputeShader> mHzCullCS = nullptr;

	ComPtr<ID3D11Buffer> mConstantBuffer = nullptr;
	ComPtr<ID3D11Buffer> mCullConstantBuffer = nullptr;	

	ComPtr<ID3D11SamplerState> mPointClampSampler = nullptr;

	// AABB
	ComPtr<ID3D11ShaderResourceView> mAABBSRV = nullptr;
	ComPtr<ID3D11Buffer> mAABBBuffer = nullptr;
	uint32 mAllocatedAABBCount = 0;

	// Visibility
	ComPtr<ID3D11Buffer> mVisibilityBuffer = nullptr;
	ComPtr<ID3D11UnorderedAccessView> mVisibilityUAV = nullptr;

	// Staging
	ComPtr<ID3D11Buffer> mStagingBuffers[2] = { nullptr, nullptr };
	uint32 mCurrentStatingIndex = 0;
	bool mHasValidStagingData = false;

	void createResources(ID3D11Device* device);
	void createShader(ID3D11Device* device);
	void createCullShader(ID3D11Device* device);
	void ensureBufferCapacity(ID3D11Device* device, uint32 requiredCount); // Reallocate GPU buffer when increasing objects
};
