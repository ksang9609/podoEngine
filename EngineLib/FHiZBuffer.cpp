#include "FHiZBuffer.h"
#include <d3dcompiler.h>

void FHiZBuffer::Initialize(ID3D11Device* device)
{
	createResources(device);
	createShader(device);
}

void FHiZBuffer::Release()
{
	for (UINT i = 0; i < MipLevels; ++i)
	{
		mHzbMipUAV[i].Reset();
		mHzbMipSRV[i].Reset();
	}
	mHzbTexture.Reset();
	mHzbFullSRV.Reset();
	mHzBuildCS.Reset();
	mConstantBuffer.Reset();
	mPointClampSampler.Reset();
}

void FHiZBuffer::BuildHiZ(ID3D11DeviceContext* context, ID3D11ShaderResourceView* mainDepthSRV, uint32 screenWidth, uint32 screenHeight)
{
	if (!mainDepthSRV || !mHzBuildCS)
	{
		return;
	}

	// Binding shader and sampler
	context->CSSetShader(mHzBuildCS.Get(), nullptr, 0);
	context->CSSetSamplers(0, 1, mPointClampSampler.GetAddressOf());

	uint32 currentDstWidth = HZBWidth;
	uint32 currentDstHeight = HZBHeight;
	uint32 prevSrcWidth = screenWidth;
	uint32 prevSrcHeight = screenHeight;

	for (uint32 mip = 0; mip < MipLevels; ++mip)
	{
		// Update constant buffer
		FHiZBufferConstants cbData = {};
		cbData.DstWidth = currentDstWidth;
		cbData.DstHeight = currentDstHeight;
		cbData.InvDstWidth = 1.0f / (float)currentDstWidth;
		cbData.InvDstHeight = 1.0f / (float)currentDstHeight;
		cbData.SrcTexelSizeX = 1.0f / (float)prevSrcWidth;
		cbData.SrcTexelSizeY = 1.0f / (float)prevSrcHeight;
		cbData.IsPass0 = (mip == 0) ? 1 : 0;

		context->UpdateSubresource(mConstantBuffer.Get(), 0, nullptr, &cbData, 0, 0);
		context->CSSetConstantBuffers(0, 1, mConstantBuffer.GetAddressOf());

		// Binding Write SRV and Read UAV
		ID3D11ShaderResourceView* inputSRV = (mip == 0) ? mainDepthSRV : mHzbMipSRV[mip - 1].Get();
		context->CSSetShaderResources(0, 1, &inputSRV);
		context->CSSetUnorderedAccessViews(0, 1, mHzbMipUAV[mip].GetAddressOf(), nullptr);

		// Dispatch (16 x 16 threads) (16, 16, 1)
		uint32 threadGroupsX = (currentDstWidth + 15) / 16;
		uint32 threadGroupsY = (currentDstHeight + 15) / 16;
		context->Dispatch(threadGroupsX, threadGroupsY, 1);

		// Unbinding slot
		ID3D11ShaderResourceView* nullSRV = nullptr;
		ID3D11UnorderedAccessView* nullUAV = nullptr;
		context->CSSetShaderResources(0, 1, &nullSRV);
		context->CSSetUnorderedAccessViews(0, 1, &nullUAV, nullptr);

		// Update next Width, Height
		prevSrcWidth = currentDstWidth;
		prevSrcHeight = currentDstHeight;
		currentDstWidth = (currentDstWidth > 1) ? (currentDstWidth / 2) : 1;
		currentDstHeight = (currentDstHeight > 1) ? (currentDstHeight / 2) : 1;
	}

	// Release Shader
	context->CSSetShader(nullptr, nullptr, 0);
}

void FHiZBuffer::createResources(ID3D11Device* device)
{
	// Create R32_Float texture
	D3D11_TEXTURE2D_DESC texDesc = {};
	texDesc.Width = HZBWidth;
	texDesc.Height = HZBHeight;
	texDesc.MipLevels = MipLevels;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R32_FLOAT;
	texDesc.SampleDesc.Count = 1;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	HRESULT hr = device->CreateTexture2D(&texDesc, nullptr, &mHzbTexture);

	// Create full Mipmap SRV
	D3D11_SHADER_RESOURCE_VIEW_DESC fullSRVDesc = {};
	fullSRVDesc.Format = DXGI_FORMAT_R32_FLOAT;
	fullSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	fullSRVDesc.Texture2D.MostDetailedMip = 0;
	fullSRVDesc.Texture2D.MipLevels = MipLevels;
	hr = device->CreateShaderResourceView(mHzbTexture.Get(), &fullSRVDesc, &mHzbFullSRV);

	// Create UAV, SRV for Mipmap
	for (UINT mip = 0; mip < MipLevels; ++mip)
	{
		// UAV
		D3D11_UNORDERED_ACCESS_VIEW_DESC uavDecs = {};
		uavDecs.Format = DXGI_FORMAT_R32_FLOAT;
		uavDecs.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
		uavDecs.Texture2D.MipSlice = mip;
		hr = device->CreateUnorderedAccessView(mHzbTexture.Get(), &uavDecs, &mHzbMipUAV[mip]);

		// SRV
		D3D11_SHADER_RESOURCE_VIEW_DESC mipSRVDecs = {};
		mipSRVDecs.Format = DXGI_FORMAT_R32_FLOAT;
		mipSRVDecs.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		mipSRVDecs.Texture2D.MostDetailedMip = mip;
		mipSRVDecs.Texture2D.MipLevels = 1;
		hr = device->CreateShaderResourceView(mHzbTexture.Get(), &mipSRVDecs, &mHzbMipSRV[mip]);
	}

	// Create constatns buffer
	D3D11_BUFFER_DESC cbDesc = {};
	cbDesc.ByteWidth = sizeof(FHiZBufferConstants);
	cbDesc.Usage = D3D11_USAGE_DEFAULT;
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	hr = device->CreateBuffer(&cbDesc, nullptr, &mConstantBuffer);

	// Crate point clamp sampler
	D3D11_SAMPLER_DESC sampDesc = {};
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	hr = device->CreateSamplerState(&sampDesc, &mPointClampSampler);
}

void FHiZBuffer::createShader(ID3D11Device* device)
{
	ComPtr<ID3DBlob> csBlob;
	ComPtr<ID3DBlob> errorBlob;

	HRESULT hr = D3DCompileFromFile(
		L"Shaders/HzBuildCS.hlsl",
		nullptr, nullptr,
		"mainCS", "cs_5_0",
		0, 0,
		&csBlob, &errorBlob
	);

	hr = device->CreateComputeShader(csBlob->GetBufferPointer(), csBlob->GetBufferSize(), nullptr, &mHzBuildCS);
}
