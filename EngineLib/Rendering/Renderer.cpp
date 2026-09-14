#include "Renderer.h"

#include <fstream>
#include <filesystem>
#include <vector>
#include <directxtk/DDSTextureLoader.h>

//#include "WICTextureLoader.h"
#include "Core/Math/MathUtility.h"
#include "Editor/Console.h"
#include "Rendering/Primitives/TexturedPrimitives.h"

#pragma comment(lib, "DirectXTK.lib")
#pragma comment(lib, "dxguid.lib")

void URenderer::Create(HWND hWindow)
{
	createDeviceAndSwapChain(hWindow);
	createFrameBuffer();

	if (!createFontAtlasTexture() ||
		!createFontSamplerState() ||
		!createFontBlendState())
	{
		MessageBox(
			hWindow,
			L"폰트 공통 자원 생성에 실패했습니다.",
			L"Font initialization error",
			MB_OK | MB_ICONERROR
		);

		releaseFontTexture();
		releaseFontAtlasTexture();
	}

	createDepthStencilState();
	createStencilMarkState();
	createStencilOutlineState();
	createNoColorWriteBlendState();
	createRasterizerState();
	createShader();
	createConstantBuffer();
	createLineVertexBuffer(LINE_VERTEX_CAPACITY);
	createLineIndexBuffer(LINE_INDEX_CAPACITY);

	/* Particle */
	createParticleStates();
	createParticleVertexBuffer();
	createParticleIndexBuffer();
}

void URenderer::createDeviceAndSwapChain(HWND hWindow)
{
	D3D_FEATURE_LEVEL featurelevels[] = { D3D_FEATURE_LEVEL_11_0 };

	DXGI_SWAP_CHAIN_DESC swapchaindesc = {};
	swapchaindesc.BufferDesc.Width = 0;
	swapchaindesc.BufferDesc.Height = 0;
	swapchaindesc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	swapchaindesc.SampleDesc.Count = 1;
	swapchaindesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapchaindesc.BufferCount = 2;
	swapchaindesc.OutputWindow = hWindow;
	swapchaindesc.Windowed = TRUE;
	swapchaindesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

	UINT createDeviceFlags = 0;

#if defined(_DEBUG)
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
		D3D11_CREATE_DEVICE_BGRA_SUPPORT | createDeviceFlags,
		featurelevels, ARRAYSIZE(featurelevels), D3D11_SDK_VERSION,
		&swapchaindesc, &SwapChain, &Device, nullptr, &DeviceContext);

	SwapChain->GetDesc(&swapchaindesc);

	ViewportInfo = { 0.0f, 0.0f, (float)swapchaindesc.BufferDesc.Width, (float)swapchaindesc.BufferDesc.Height, 0.0f, 1.0f };
}

void URenderer::releaseDeviceAndSwapChain()
{
	if (DeviceContext)
	{
		DeviceContext->Flush();
	}

	if (SwapChain)
	{
		SwapChain->Release();
		SwapChain = nullptr;
	}

	if (Device)
	{
		Device->Release();
		Device = nullptr;
	}

	if (DeviceContext)
	{
		DeviceContext->Release();
		DeviceContext = nullptr;
	}
}

void URenderer::createFrameBuffer()
{
	SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&FrameBuffer);

	D3D11_RENDER_TARGET_VIEW_DESC framebufferRTVdesc = {};
	framebufferRTVdesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
	framebufferRTVdesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

	Device->CreateRenderTargetView(FrameBuffer, &framebufferRTVdesc, &FrameBufferRTV);
}

void URenderer::releaseFrameBuffer()
{
	if (FrameBuffer)
	{
		FrameBuffer->Release();
		FrameBuffer = nullptr;
	}

	if (FrameBufferRTV)
	{
		FrameBufferRTV->Release();
		FrameBufferRTV = nullptr;
	}
}

bool URenderer::createFontAtlasTexture()
{
	if (Device == nullptr)
	{
		return false;
	}

	// 함수가 다시 호출되는 경우 기존 텍스처 해제
	
	releaseFontAtlasTexture();

	// 추후 동적으로 텍스쳐 로드하자 
	HRESULT hr = DirectX::CreateDDSTextureFromFile(
		Device,
		L"Assets/Fonts/EnglishBigFontAtlas.dds",
		nullptr,
		&FontAtlasShaderResoruceView
	);

	if (FAILED(hr))
	{
		FontAtlasShaderResoruceView = nullptr;
		return false;
	}

	return true;
}

void URenderer::releaseFontAtlasTexture()
{
	if (FontAtlasShaderResoruceView != nullptr)
	{
		FontAtlasShaderResoruceView->Release();
		FontAtlasShaderResoruceView = nullptr;
	}
}

bool URenderer::createFontSamplerState()
{
	if (!Device)
		return false;

	if (FontSamplerState)
	{
		FontSamplerState->Release();
		FontSamplerState = nullptr;
	}

	D3D11_SAMPLER_DESC desc = {};
	desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	desc.MaxAnisotropy = 1;
	desc.MaxLOD = D3D11_FLOAT32_MAX;

	return SUCCEEDED(Device->CreateSamplerState(&desc, &FontSamplerState)
	);
}

bool URenderer::createFontBlendState()
{
	if (!Device)
		return false;

	if (FontBlendState)
	{
		FontBlendState->Release();
		FontBlendState = nullptr;
	}

	D3D11_BLEND_DESC desc = {};
	auto& rt = desc.RenderTarget[0];

	rt.BlendEnable = TRUE;
	rt.SrcBlend = D3D11_BLEND_SRC_ALPHA;
	rt.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	rt.BlendOp = D3D11_BLEND_OP_ADD;

	rt.SrcBlendAlpha = D3D11_BLEND_ONE;
	rt.DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
	rt.BlendOpAlpha = D3D11_BLEND_OP_ADD;
	rt.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	return SUCCEEDED(Device->CreateBlendState(&desc, &FontBlendState));
}

bool URenderer::createParticleStates()
{
	if (!Device)
		return false;
	if (ParticleBlendState)
	{
		ParticleBlendState->Release();
		ParticleBlendState = nullptr;
	}
	D3D11_BLEND_DESC desc = {};
	auto& rt = desc.RenderTarget[0];

	rt.BlendEnable = TRUE;
	rt.SrcBlend = D3D11_BLEND_SRC_ALPHA;
	rt.DestBlend = D3D11_BLEND_ONE;
	rt.BlendOp = D3D11_BLEND_OP_ADD;

	rt.SrcBlendAlpha = D3D11_BLEND_ONE;
	rt.DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
	rt.BlendOpAlpha = D3D11_BLEND_OP_ADD;
	rt.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	if (ParticleSamplerState)
	{
		ParticleSamplerState->Release();
		ParticleSamplerState = nullptr;
	}

	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	return SUCCEEDED(Device->CreateSamplerState(&samplerDesc, &ParticleSamplerState)) &&
		SUCCEEDED(Device->CreateBlendState(&desc, &ParticleBlendState));
}

// 인스턴스 사용하여 렌더링(텍스쳐 X)
bool URenderer::RenderSimpleInstanced(
	ID3D11Buffer* vertexBuffer,
	ID3D11Buffer* indexBuffer,
	UINT indexCount,
	const FInstanceData* instances,
	UINT instanceCount)
{
	if (instanceCount == 0)
		return true;

	if (!DeviceContext ||
		!vertexBuffer ||
		!instances ||
		!indexBuffer ||
		indexCount == 0 ||
		!ConstantBuffer ||
		!InstancedVertexShader ||
		!InstancedInputLayout ||
		!SimplePixelShader)
	{
		return false;
	}

	if (!EnsureInstanceCapacity(instanceCount))
		return false;

	// CPU의 인스턴스 배열을 GPU 버퍼에 복사
	// Map / Unmap은 “CPU가 쓸 수 있게 잠깐 문 열어주는 것”
	D3D11_MAPPED_SUBRESOURCE mapped{};

	HRESULT hr = DeviceContext->Map(InstanceBuffer,	0,	D3D11_MAP_WRITE_DISCARD, 0,	&mapped);

	if (FAILED(hr))
	{
		return false;
	}

	std::memcpy(mapped.pData, instances, static_cast<size_t>(instanceCount) * sizeof(FInstanceData));
	DeviceContext->Unmap(InstanceBuffer, 0);

	const UINT vertexStride = sizeof(FVertexSimple);
	const UINT instanceStride = sizeof(FInstanceData);
	UINT offset = 0;

	ID3D11Buffer* vbs[2] = { vertexBuffer, InstanceBuffer }; // VertexBuffer와 InstanceBuffer 각각의 슬롯 0,1에 삽입
	UINT strides[2] = { vertexStride, instanceStride };
	UINT offsets[2] = { 0, 0 };
	DeviceContext->IASetVertexBuffers(0, 2, vbs, strides, offsets);


	// 인덱스 버퍼는 정점 버퍼 슬롯과 별도로 연결
	DeviceContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);


	// indexCount: 인스턴스 하나를 그리는 데 사용할 인덱스 개수
	DeviceContext->DrawIndexedInstanced(indexCount,	instanceCount, 0, 0, 0);

	return true;

}

// 인스턴스 버퍼의 크기를 확인하고 필요하면 증가
bool URenderer::EnsureInstanceCapacity(UINT count)
{
	if (count == 0)
		return true;

	const UINT maxCount = 100000; // 10만개(임의로 정함)
	const UINT instanceSize = static_cast<UINT>(sizeof(FInstanceData));

	// 기존 버퍼가 충분하면 그대로 사용한다.
	if (InstanceBuffer && count <= InstanceCapacity)
		return true;

	// 최대용량 초과시
	if (count > maxCount)
		return false;

	if (!Device)
		return false;

	UINT newCapacity = InstanceCapacity > 0 ? InstanceCapacity : 256;

	while (newCapacity < count)
	{
		if (newCapacity > maxCount / 2)
		{
			newCapacity = count;
			break;
		}

		newCapacity *= 2;
	}

	D3D11_BUFFER_DESC desc{};
	desc.ByteWidth = newCapacity * instanceSize;
	desc.Usage = D3D11_USAGE_DYNAMIC; // 동적
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER; // 인스턴스 버퍼도 일단 버텍스 버퍼
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	ID3D11Buffer* newBuffer = nullptr;

	HRESULT hr = Device->CreateBuffer(&desc, nullptr, &newBuffer);

	if (FAILED(hr))
	{
		return false;
	}

	// 새 버퍼 생성이 성공한 뒤 기존 버퍼를 해제한다.
	if (InstanceBuffer)
		InstanceBuffer->Release();

	InstanceBuffer = newBuffer;
	InstanceCapacity = newCapacity;

	return true;
}

/// 
ID3D11Buffer* URenderer::CreateVertexBuffer(FVertexSimple* vertices, UINT ByteWidth)
{
	UINT numVertices = ByteWidth / sizeof(FVertexSimple);

	D3D11_BUFFER_DESC vertexbufferdesc = {};
	vertexbufferdesc.ByteWidth = ByteWidth;
	vertexbufferdesc.Usage = D3D11_USAGE_IMMUTABLE;
	vertexbufferdesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA vertexbufferSRD = { vertices };

	ID3D11Buffer* vertexBuffer;
	Device->CreateBuffer(&vertexbufferdesc, &vertexbufferSRD, &vertexBuffer);

	return vertexBuffer;
}

// 텍스쳐 매핑시에 사용
ID3D11Buffer* URenderer::CreateVertexBuffer(const FVertexTextured* vertices, UINT byteWidth)
{
	D3D11_BUFFER_DESC desc = {};
	desc.ByteWidth = byteWidth;
	desc.Usage = D3D11_USAGE_IMMUTABLE;
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA data = {};
	data.pSysMem = vertices;

	ID3D11Buffer* buffer = nullptr;
	if (FAILED(Device->CreateBuffer(&desc, &data, &buffer)))
		return nullptr;

	return buffer;
}

void URenderer::ReleaseVertexBuffer(ID3D11Buffer* vertexBuffer)
{
	vertexBuffer->Release();
}

// 선분은 매 프레임 내용이 바뀌므로 IMMUTABLE로는 만들 수 없다.
// DYNAMIC + CPU_ACCESS_WRITE 라야 Map으로 덮어쓸 수 있다. (상수 버퍼와 같은 조합)
void URenderer::createLineVertexBuffer(uint32 maxVertices)
{
	D3D11_BUFFER_DESC vertexbufferdesc = {};
	vertexbufferdesc.ByteWidth = maxVertices * sizeof(FVertexSimple);
	vertexbufferdesc.Usage = D3D11_USAGE_DYNAMIC;
	vertexbufferdesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexbufferdesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	if (SUCCEEDED(Device->CreateBuffer(&vertexbufferdesc, nullptr, &LineVertexBuffer)))
	{
		LineVertexCapacity = maxVertices;
	}
}

void URenderer::releaseLineVertexBuffer()
{
	if (LineVertexBuffer)
	{
		LineVertexBuffer->Release();
		LineVertexBuffer = nullptr;
	}

	LineVertexCapacity = 0;
}

void URenderer::releaseFontBuffers()
{
	if (FontTextureBuffer)
	{
		FontTextureBuffer->Release();
		FontTextureBuffer = nullptr;
	}

	if (FontIndexBuffer)
	{
		FontIndexBuffer->Release();
		FontIndexBuffer = nullptr;
	}
}

void URenderer::createLineIndexBuffer(uint32 maxIndices)
{
	D3D11_BUFFER_DESC indexbufferdesc = {};
	indexbufferdesc.ByteWidth = maxIndices * sizeof(uint32);
	indexbufferdesc.Usage = D3D11_USAGE_DYNAMIC;
	indexbufferdesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexbufferdesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	if (SUCCEEDED(Device->CreateBuffer(&indexbufferdesc, nullptr, &LineIndexBuffer)))
	{
		LineIndexCapacity = maxIndices;
	}
}

void URenderer::createParticleVertexBuffer()
{
	D3D11_BUFFER_DESC vertexbufferdesc = {};
	// Assume particle is a quad
	vertexbufferdesc.ByteWidth = 4 * sizeof(FVertexTextured);
	vertexbufferdesc.Usage = D3D11_USAGE_DYNAMIC;
	vertexbufferdesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexbufferdesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	Device->CreateBuffer(&vertexbufferdesc, nullptr, &ParticleVertexBuffer);
}

void URenderer::createParticleIndexBuffer()
{
	D3D11_BUFFER_DESC indexbufferdesc = {};
	indexbufferdesc.ByteWidth = 6 * sizeof(uint32);
	indexbufferdesc.Usage = D3D11_USAGE_DYNAMIC;
	indexbufferdesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexbufferdesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	Device->CreateBuffer(&indexbufferdesc, nullptr, &ParticleIndexBuffer);
}

void URenderer::releaseLineIndexBuffer()
{
	if (LineIndexBuffer)
	{
		LineIndexBuffer->Release();
		LineIndexBuffer = nullptr;
	}

	LineIndexCapacity = 0;
}

void URenderer::createRasterizerState()
{
	D3D11_RASTERIZER_DESC rasterizerdesc[2] = {};
	rasterizerdesc[0].FillMode = D3D11_FILL_SOLID;
	rasterizerdesc[0].CullMode = D3D11_CULL_BACK;
	rasterizerdesc[0].DepthClipEnable = TRUE;

	rasterizerdesc[1].FillMode = D3D11_FILL_WIREFRAME;
	rasterizerdesc[1].CullMode = D3D11_CULL_NONE;
	rasterizerdesc[1].DepthClipEnable = TRUE;

	Device->CreateRasterizerState(&rasterizerdesc[0], &RasterizerState[0]);
	Device->CreateRasterizerState(&rasterizerdesc[1], &RasterizerState[1]);
}

void URenderer::releaseRasterizerState()
{
	for (int i = 0; i < 2; ++i)
	{
		if (RasterizerState[i])
		{
			RasterizerState[i]->Release();
			RasterizerState[i] = nullptr;
		}
	}
}
void URenderer::Release()
{
	releaseRasterizerState();

	DeviceContext->OMSetRenderTargets(0, nullptr, nullptr);

	releaseDepthStencilBuffer();
	releaseDepthStencilState();
	releaseBlendState();
	releaseFrameBuffer();
	releaseLineVertexBuffer();
	releaseLineIndexBuffer();
	releaseConstantBuffer();
	releaseShader();

	// 테스트

	if (CubeIndexBuffer)
	{
		CubeIndexBuffer->Release();
		CubeIndexBuffer = nullptr;
	}
	releaseFontAtlasTexture();
	releaseFontTexture();
	releaseFontBuffers();
	//ReleaseTestTexture();
	releaseDeviceAndSwapChain();

	if (InstanceBuffer)
	{
		InstanceBuffer->Release();
		InstanceBuffer = nullptr;
	}

	/* Particle */
	if (ParticleVertexBuffer)
	{
		ParticleVertexBuffer->Release();
		ParticleVertexBuffer = nullptr;
	}
	if (ParticleIndexBuffer)
	{
		ParticleIndexBuffer->Release();
		ParticleIndexBuffer = nullptr;
	}
}

void URenderer::SwapBuffer()
{
	SwapChain->Present(1, 0);
}

void URenderer::createShader()
{
	ID3DBlob* vertexshaderCSO;
	ID3DBlob* pixelshaderCSO;
	ID3DBlob* LinevertexshaderCSO;
	ID3DBlob* LinepixelshaderCSO;
	ID3DBlob* primitiveTextureVertexShaderCSO;
	ID3DBlob* primitiveTexturePixelShaderCSO;
	ID3DBlob* instancedVertexShaderCS0;
	ID3DBlob* fontVertexShaderCSO;
	ID3DBlob* fontPixelShaderCSO;



	D3DCompileFromFile(L"Shaders/ShaderW0.hlsl", nullptr, nullptr, "mainVS", "vs_5_0", 0, 0, &vertexshaderCSO, nullptr);

	Device->CreateVertexShader(vertexshaderCSO->GetBufferPointer(), vertexshaderCSO->GetBufferSize(), nullptr, &SimpleVertexShader);

	D3DCompileFromFile(L"Shaders/ShaderW0.hlsl", nullptr, nullptr, "mainPS", "ps_5_0", 0, 0, &pixelshaderCSO, nullptr);

	Device->CreatePixelShader(pixelshaderCSO->GetBufferPointer(), pixelshaderCSO->GetBufferSize(), nullptr, &SimplePixelShader);

	D3DCompileFromFile(L"Shaders/ShaderLine.hlsl", nullptr, nullptr, "mainVS", "vs_5_0", 0, 0, &LinevertexshaderCSO, nullptr);

	Device->CreateVertexShader(LinevertexshaderCSO->GetBufferPointer(), LinevertexshaderCSO->GetBufferSize(), nullptr, &LineSimpleVertexShader);

	D3DCompileFromFile(L"Shaders/ShaderLine.hlsl", nullptr, nullptr, "mainPS", "ps_5_0", 0, 0, &LinepixelshaderCSO, nullptr);

	Device->CreatePixelShader(LinepixelshaderCSO->GetBufferPointer(), LinepixelshaderCSO->GetBufferSize(), nullptr, &LineSimplePixelShader);

	D3DCompileFromFile(L"Shaders/ShaderTexture.hlsl",nullptr,	nullptr,"mainVS","vs_5_0",0,0,	&primitiveTextureVertexShaderCSO, nullptr);

	Device->CreateVertexShader(primitiveTextureVertexShaderCSO->GetBufferPointer(), primitiveTextureVertexShaderCSO->GetBufferSize(), nullptr,	&PrimitiveTextureVertexShader);

	D3DCompileFromFile(L"Shaders/ShaderTexture.hlsl", nullptr,	nullptr,"mainPS", "ps_5_0",	0,	0,	&primitiveTexturePixelShaderCSO, nullptr);

	Device->CreatePixelShader(primitiveTexturePixelShaderCSO->GetBufferPointer(), primitiveTexturePixelShaderCSO->GetBufferSize(), nullptr, &PrimitiveTexturePixelShader);

	// 인스턴싱
	D3DCompileFromFile(L"Shaders/ShaderW0.hlsl", nullptr, nullptr, "mainVSInstanced", "vs_5_0", 0, 0, &instancedVertexShaderCS0, nullptr);

	Device->CreateVertexShader(instancedVertexShaderCS0->GetBufferPointer(), instancedVertexShaderCS0->GetBufferSize(), nullptr, &InstancedVertexShader);

	D3DCompileFromFile(L"Shaders/ShaderFont.hlsl", nullptr, nullptr, "mainVS", "vs_5_0", 0, 0, &fontVertexShaderCSO, nullptr);

	Device->CreateVertexShader(fontVertexShaderCSO->GetBufferPointer(), fontVertexShaderCSO->GetBufferSize(), nullptr, &FontVertexShader);

	D3DCompileFromFile(L"Shaders/ShaderFont.hlsl", nullptr, nullptr, "mainPS", "ps_5_0", 0, 0, &fontPixelShaderCSO, nullptr);

	Device->CreatePixelShader(fontPixelShaderCSO->GetBufferPointer(), fontPixelShaderCSO->GetBufferSize(), nullptr, &FontPixelShader);


	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	D3D11_INPUT_ELEMENT_DESC Linelayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	D3D11_INPUT_ELEMENT_DESC primitiveTextureLayout[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,	0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0	}// float u, v;    // 12바이트 위치부터 시작
	};

	const D3D11_INPUT_ELEMENT_DESC layoutInstanced[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,  0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT,  0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },

		 // 슬롯 1: 인스턴스의 World 행렬(행렬을 한꺼번에 넣는 건 불가능, 한줄 씩 넣는다)
		{ "INSTANCE_WORLD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "INSTANCE_WORLD", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1 }, // 16: 내부 오프셋
		{ "INSTANCE_WORLD", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 32, D3D11_INPUT_PER_INSTANCE_DATA, 1 }, // 32: 내부 오프셋
		{ "INSTANCE_WORLD", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 48, D3D11_INPUT_PER_INSTANCE_DATA, 1 }, // 48: 내부 오프셋

		//// 슬롯 1: 인스턴스의 Tint
		{ "INSTANCE_TINT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 64, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
	};


	Device->CreateInputLayout(layout, ARRAYSIZE(layout), vertexshaderCSO->GetBufferPointer(), vertexshaderCSO->GetBufferSize(), &SimpleInputLayout);
	Device->CreateInputLayout(Linelayout, ARRAYSIZE(Linelayout), LinevertexshaderCSO->GetBufferPointer(), LinevertexshaderCSO->GetBufferSize(), &LineSimpleInputLayout);
	Device->CreateInputLayout(primitiveTextureLayout,ARRAYSIZE(primitiveTextureLayout), primitiveTextureVertexShaderCSO->GetBufferPointer(), primitiveTextureVertexShaderCSO->GetBufferSize(), &PrimitiveTextureLayout);
	Device->CreateInputLayout(primitiveTextureLayout, ARRAYSIZE(primitiveTextureLayout), fontVertexShaderCSO->GetBufferPointer(), fontVertexShaderCSO->GetBufferSize(), &FontInputLayout);

	Device->CreateInputLayout(layoutInstanced, ARRAYSIZE(layoutInstanced), instancedVertexShaderCS0->GetBufferPointer(), instancedVertexShaderCS0->GetBufferSize(), &InstancedInputLayout);

	StrideSimple = sizeof(FVertexSimple);
	StrideTextured = sizeof(FVertexTextured);

	vertexshaderCSO->Release();
	pixelshaderCSO->Release();
	LinevertexshaderCSO->Release();
	LinepixelshaderCSO->Release();
	primitiveTextureVertexShaderCSO->Release();
	primitiveTexturePixelShaderCSO->Release();
	instancedVertexShaderCS0->Release();
	fontVertexShaderCSO->Release();
	fontPixelShaderCSO->Release();
}

// Sampler State 설정
void URenderer::CreateSamplerState(ID3D11SamplerState** outSamplerState)
{
	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	Device->CreateSamplerState(&samplerDesc, outSamplerState);
}

void URenderer::releaseShader()
{
	/* Simple Shader */
	if (SimpleInputLayout)
	{
		SimpleInputLayout->Release();
		SimpleInputLayout = nullptr;
	}

	if (SimplePixelShader)
	{
		SimplePixelShader->Release();
		SimplePixelShader = nullptr;
	}

	if (SimpleVertexShader)
	{
		SimpleVertexShader->Release();
		SimpleVertexShader = nullptr;
	}

	/* Line Shader */
	if (LineSimpleInputLayout)
	{
		LineSimpleInputLayout->Release();
		LineSimpleInputLayout = nullptr;
	}

	if (LineSimplePixelShader)
	{
		LineSimplePixelShader->Release();
		LineSimplePixelShader = nullptr;
	}

	if (LineSimpleVertexShader)
	{
		LineSimpleVertexShader->Release();
		LineSimpleVertexShader = nullptr;
	}

	/* Texture Shader */
	if (FontInputLayout)
	{
		FontInputLayout->Release();
		FontInputLayout = nullptr;
	}

	if (FontPixelShader)
	{
		FontPixelShader->Release();
		FontPixelShader = nullptr;
	}

	if (FontVertexShader)
	{
		FontVertexShader->Release();
		FontVertexShader = nullptr;
	}

	/* Primitive Texture Shader */
	if (PrimitiveTextureLayout)
	{
		PrimitiveTextureLayout->Release();
		PrimitiveTextureLayout = nullptr;
	}

	if (PrimitiveTexturePixelShader)
	{
		PrimitiveTexturePixelShader->Release();
		PrimitiveTexturePixelShader = nullptr;
	}

	if (PrimitiveTextureVertexShader)
	{
		PrimitiveTextureVertexShader->Release();
		PrimitiveTextureVertexShader = nullptr;
	}

	/*Instancing*/
	if (InstancedInputLayout)
	{
		InstancedInputLayout->Release();
		InstancedInputLayout = nullptr;
	}

	if (InstancedVertexShader)
	{
		InstancedVertexShader->Release();
		InstancedVertexShader = nullptr;
	}
	/* Font Shader */
	if (FontVertexShader)
	{
		FontVertexShader->Release();
		FontVertexShader = nullptr;
	}

	if (FontPixelShader)
	{
		FontPixelShader->Release();
		FontPixelShader = nullptr;
	}

	if (FontInputLayout)
	{
		FontInputLayout->Release();
		FontInputLayout = nullptr;
	}
}

// 개별 이미지: 지정된 파일을 로딩해서 결과를 반환
bool URenderer::LoadTexture(const wchar_t* texturePath, ID3D11ShaderResourceView** outSRV)
{
	if (!Device || !texturePath || texturePath[0] == L'\0' || !outSRV)
	{
		OutputDebugStringA("LoadTexture: invalid argument or Device.\n");
		return false;
	}

	// 임시 포인터로 로딩해서 실패 시 기존 텍스처를 보존
	ID3D11ShaderResourceView* loadedSRV = nullptr;

	const HRESULT hr = DirectX::CreateDDSTextureFromFile(
		Device,
		texturePath,
		nullptr,
		&loadedSRV);

	if (FAILED(hr))
	{
		if (loadedSRV)
			loadedSRV->Release();

		OutputDebugStringW(L"LoadTexture failed: ");
		OutputDebugStringW(texturePath);
		OutputDebugStringW(L"\n");

		return false;
	}

	// 성공한 경우에만 기존 텍스처를 교체
	if (*outSRV)
		(*outSRV)->Release();

	*outSRV = loadedSRV;
	return true;
}

// Prepare global rendering state for a new frame
void URenderer::Prepare(bool bWireFrame)
{
	DeviceContext->ClearRenderTargetView(FrameBufferRTV, ClearColor);

	//매 프레임 깊이 버퍼를 1.0(가장 먼 값)으로 초기화
	DeviceContext->ClearDepthStencilView(DepthStencilView,
		D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	DeviceContext->RSSetViewports(1, &ViewportInfo);

	//DeviceContext->RSSetState(RasterizerState[bWireFrame ? 1 : 0]);
	mbWireFrame = bWireFrame;

	//세 번째 인자에 nullptr 대신 DSV를 넘긴다
	DeviceContext->OMSetRenderTargets(1, &FrameBufferRTV, DepthStencilView);
	//깊이 테스트 규칙 적용
	DeviceContext->OMSetDepthStencilState(DepthStencilState, 0);
	DeviceContext->OMSetBlendState(nullptr, nullptr, 0xffffffff);
}

void URenderer::PrepareSimplePrimitive()
{
	prepareSimpleShader();

	DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	DeviceContext->RSSetState(RasterizerState[mbWireFrame ? 1 : 0]);
	DeviceContext->OMSetBlendState(nullptr, nullptr, 0xffffffff);
}

void URenderer::PrepareTexturedPrimitive()
{
	prepareTextureShader();

	DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	DeviceContext->RSSetState(RasterizerState[mbWireFrame ? 1 : 0]);
	DeviceContext->OMSetBlendState(nullptr, nullptr, 0xffffffff);
}

void URenderer::PrepareLine()
{
	prepareLineShader();

	DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

	DeviceContext->RSSetState(RasterizerState[mbWireFrame ? 1 : 0]);
	DeviceContext->OMSetBlendState(nullptr, nullptr, 0xffffffff);
}

void URenderer::PrepareFont()
{
	prepareFontShader();

	DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Always render solid
	DeviceContext->RSSetState(RasterizerState[0]);
	DeviceContext->OMSetBlendState(FontBlendState, nullptr, 0xffffffff);
}

void URenderer::PrepareGizmo()
{
	prepareSimpleShader();

	DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Always render solid
	DeviceContext->RSSetState(RasterizerState[0]);
	DeviceContext->OMSetBlendState(nullptr, nullptr, 0xffffffff);
}

void URenderer::PrepareParticle()
{
	prepareTextureShader();

	DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Always render solid
	DeviceContext->RSSetState(RasterizerState[0]);
	DeviceContext->OMSetBlendState(ParticleBlendState, nullptr, 0xffffffff);
}

void URenderer::PrepareHighlight()
{
	prepareSimpleShader();

	DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Always render solid
	DeviceContext->RSSetState(RasterizerState[0]);
	DeviceContext->OMSetBlendState(nullptr, nullptr, 0xffffffff);
}

void URenderer::PrepareSimpleInstanced()
{
	prepareInstancedShader();

	DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	DeviceContext->RSSetState(RasterizerState[mbWireFrame ? 1 : 0]);
	DeviceContext->OMSetBlendState(nullptr, nullptr, 0xffffffff);
}

void URenderer::prepareInstancedShader()
{
	DeviceContext->VSSetShader(InstancedVertexShader, nullptr, 0);
	DeviceContext->PSSetShader(SimplePixelShader, nullptr, 0);
	DeviceContext->IASetInputLayout(InstancedInputLayout);

	if (ConstantBuffer)
	{
		DeviceContext->VSSetConstantBuffers(0, 1, &ConstantBuffer);
	}
}

void URenderer::prepareSimpleShader()
{
	DeviceContext->VSSetShader(SimpleVertexShader, nullptr, 0);
	DeviceContext->PSSetShader(SimplePixelShader, nullptr, 0);
	DeviceContext->IASetInputLayout(SimpleInputLayout);

	if (ConstantBuffer)
	{
		DeviceContext->VSSetConstantBuffers(0, 1, &ConstantBuffer);
	}
}

void URenderer::prepareTextureShader()
{
	DeviceContext->VSSetShader(PrimitiveTextureVertexShader, nullptr, 0);
	DeviceContext->PSSetShader(PrimitiveTexturePixelShader, nullptr, 0);
	DeviceContext->IASetInputLayout(PrimitiveTextureLayout);

	if (ConstantBuffer)
	{
		DeviceContext->VSSetConstantBuffers(0, 1, &ConstantBuffer);
	}
}

void URenderer::prepareLineShader()
{
	DeviceContext->VSSetShader(LineSimpleVertexShader, nullptr, 0);
	DeviceContext->PSSetShader(LineSimplePixelShader, nullptr, 0);
	DeviceContext->IASetInputLayout(LineSimpleInputLayout);

	if (ConstantBuffer)
	{
		DeviceContext->VSSetConstantBuffers(0, 1, &ConstantBuffer);
	}
}

void URenderer::prepareFontShader()
{
	DeviceContext->VSSetShader(FontVertexShader, nullptr, 0);
	DeviceContext->PSSetShader(FontPixelShader, nullptr, 0);
	DeviceContext->IASetInputLayout(FontInputLayout);
	if (ConstantBuffer)
	{
		DeviceContext->VSSetConstantBuffers(0, 1, &ConstantBuffer);
		DeviceContext->PSSetConstantBuffers(0, 1, &ConstantBuffer);
	}
}

void URenderer::RenderSimplePrimitive(ID3D11Buffer* pBuffer, UINT numVertices)
{
	UINT offset = 0;
	// Bind the vertex buffer
	DeviceContext->IASetVertexBuffers(0, 1, &pBuffer, &StrideSimple, &offset);
	DeviceContext->Draw(numVertices, 0);
}

void URenderer::RenderTexturePrimitive(ID3D11Buffer* pBuffer, UINT numVertices,
	ID3D11ShaderResourceView* texture, ID3D11SamplerState* samplerState, ID3D11Buffer* indexBuffer, UINT indexCount)
{
	UINT offset = 0;
	// Bind the vertex buffer
	DeviceContext->IASetVertexBuffers(0, 1, &pBuffer, &StrideTextured, &offset);

	// Bind the texture resource
	DeviceContext->PSSetShaderResources(0, 1, &texture);
	DeviceContext->PSSetSamplers(0, 1, &samplerState);


	//DeviceContext->Draw(numVertices, 0);


	DeviceContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	// 임시 큐브
	if (indexBuffer)
		DeviceContext->DrawIndexed(indexCount, 0, 0);
	else
		DeviceContext->Draw(numVertices, 0);
}


//void URenderer::RenderTexturedPrimitive(ID3D11Buffer* vertexBuffer,	UINT numVertices)
//{
//	if (!DeviceContext ||
//		!vertexBuffer ||
//		numVertices == 0 ||
//		!ConstantBuffer ||
//		!PrimitiveTextureVertexShader ||
//		!PrimitiveTexturePixelShader ||
//		!PrimitiveTextureLayout ||
//		!PrimitiveTextureSRV ||
//		!PrimitiveTextureSampler ||
//		!DepthStencilState)
//	{
//		return;
//	}
//
//	// 위치 + UV 정점 버퍼 연결
//	const UINT stride = sizeof(FVertexTextured);
//	const UINT offset = 0;
//
//	DeviceContext->IASetVertexBuffers(
//		0, 1, &vertexBuffer, &stride, &offset);
//
//	DeviceContext->IASetInputLayout(PrimitiveTextureLayout);
//
//	DeviceContext->IASetPrimitiveTopology(
//		D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
//
//	// 텍스처용 셰이더 연결
//	DeviceContext->VSSetShader(PrimitiveTextureVertexShader, nullptr, 0);
//
//	DeviceContext->PSSetShader(PrimitiveTexturePixelShader, nullptr, 0);
//
//	//  GraphicsManager에서 갱신한 변환 행렬 연결
//	DeviceContext->VSSetConstantBuffers(0, 1, &ConstantBuffer);
//
//	// HLSL의 t0에 텍스처, s0에 샘플러 연결
//	DeviceContext->PSSetShaderResources(0, 1, &PrimitiveTextureSRV);
//
//	DeviceContext->PSSetSamplers(0, 1, &PrimitiveTextureSampler);
//
//	// 일반 불투명 Primitive용 상태 설정
//	DeviceContext->OMSetDepthStencilState(DepthStencilState, 0);
//
//	DeviceContext->OMSetBlendState(	nullptr, nullptr, 0xffffffff);
//
//	// 그리기
//	DeviceContext->Draw(numVertices, 0);
//
//	//  사용한 텍스처와 샘플러 연결 해제
//	ID3D11ShaderResourceView* nullSRV = nullptr;
//	ID3D11SamplerState* nullSampler = nullptr;
//
//	DeviceContext->PSSetShaderResources(0, 1, &nullSRV);
//
//	DeviceContext->PSSetSamplers(0, 1, &nullSampler);
//
//	// 기존 색상 셰이더로 돌아가기
//	PrepareSimpleShader();
//}

void URenderer::RenderFontTexture(uint32 numCharacter)
{
	UINT offset = 0;
	// Bind the vertex buffer
	DeviceContext->IASetVertexBuffers(0, 1, &FontTextureBuffer, &StrideTextured, &offset);

	// Bind the texture resource
	DeviceContext->PSSetShaderResources(0, 1, &FontAtlasShaderResoruceView);
	DeviceContext->PSSetSamplers(0, 1, &FontSamplerState);

	// Bind the index buffer
	DeviceContext->IASetIndexBuffer(FontIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	DeviceContext->DrawIndexed(numCharacter * 6, 0, 0);
}

void URenderer::RenderParticle(ID3D11ShaderResourceView* texture)
{
	if (!ParticleVertexBuffer || texture == nullptr) return;
	
	UINT offset = 0;

	// Bind the vertex buffer
	DeviceContext->IASetVertexBuffers(0, 1, &ParticleVertexBuffer, &StrideTextured, &offset);

	// Bind the texture resource
	DeviceContext->PSSetShaderResources(0, 1, &texture);
	DeviceContext->PSSetSamplers(0, 1, &ParticleSamplerState);

	// Bind the index buffer
	DeviceContext->IASetIndexBuffer(ParticleIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	DeviceContext->DrawIndexed(6, 0, 0);
}

// 쌓아둔 선분 전체를 한 번의 Draw로 그린다.
// 토폴로지를 바꾸므로 반드시 이 함수 안에서 되돌린다. 안 그러면 뒤에 그리는 것들이 전부 깨진다.
void URenderer::RenderLines(const FVertexSimple* vertices, uint32 numVertices, const uint32* indices, uint32 numindices)
{
	if (!LineVertexBuffer || vertices == nullptr || numVertices == 0) return;

	if (numVertices > LineVertexCapacity)
	{
		numVertices = LineVertexCapacity;   // 넘치면 자른다. 늘리려면 CreateLineVertexBuffer의 인자를 키운다
	}

	// WRITE_DISCARD: 이전 내용을 버리고 새 메모리를 받는다.
	// GPU가 지난 프레임 데이터를 아직 읽고 있어도 CPU가 기다리지 않는다.
	D3D11_MAPPED_SUBRESOURCE lineBufferMSR;
	if (FAILED(DeviceContext->Map(LineVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &lineBufferMSR)))
	{
		return;
	}
	memcpy(lineBufferMSR.pData, vertices, numVertices * sizeof(FVertexSimple));
	DeviceContext->Unmap(LineVertexBuffer, 0);

	if (!LineIndexBuffer || indices == nullptr || numindices == 0) return;

	if (numindices > LineIndexCapacity)
	{
		numindices = LineIndexCapacity;   // 넘치면 자른다. 늘리려면 CreateLineIndexBuffer의 인자를 키운다
	}

	// WRITE_DISCARD: 이전 내용을 버리고 새 메모리를 받는다.
	// GPU가 지난 프레임 데이터를 아직 읽고 있어도 CPU가 기다리지 않는다.
	D3D11_MAPPED_SUBRESOURCE lineBufferMSRI;
	if (FAILED(DeviceContext->Map(LineIndexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &lineBufferMSRI)))
	{
		return;
	}
	memcpy(lineBufferMSRI.pData, indices, numindices * sizeof(uint32));
	DeviceContext->Unmap(LineIndexBuffer, 0);

	// 직전에 메시 버퍼가 물려 있으므로 갈아끼워야 한다
	UINT offset = 0;
	DeviceContext->IASetVertexBuffers(0, 1, &LineVertexBuffer, &StrideSimple, &offset);
	DeviceContext->IASetIndexBuffer(LineIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	DeviceContext->DrawIndexed(numindices, 0, 0);
}

void URenderer::RenderHighlight(ID3D11Buffer* pBuffer, uint32 Num, FMatrix mViewProjectionMatrix, FMatrix OutlineMatrix, const FMatrix originalMatrix)
{
	// (a) 스텐실에 1 마킹. 색은 쓰지 않으므로 화면 변화 없음.
	//     다른 오브젝트에 가려진 부분도 반드시 마킹해야 한다. 여기서 빠지면
	//     (b)의 != 1 조건을 통과해 버려서 겹친 영역 전체가 단색으로 칠해진다.
	DeviceContext->OMSetBlendState(NoColorWriteBlendState, nullptr, 0xffffffff);
	DeviceContext->OMSetDepthStencilState(StencilMarkState, 1);
	UpdateConstant(originalMatrix, mViewProjectionMatrix);
	RenderSimplePrimitive(pBuffer, Num);

	// (b) 확대판을 단색으로. 스텐실 != 1 인 곳만 통과 -> 테두리
	DeviceContext->OMSetBlendState(nullptr, nullptr, 0xffffffff);
	DeviceContext->OMSetDepthStencilState(StencilOutlineState, 1);
	UpdateConstant(OutlineMatrix, mViewProjectionMatrix, FVector4(1.f, 0.6f, 0.f, 1.f));
	RenderSimplePrimitive(pBuffer, Num);

	// (c) 원상복구
	DeviceContext->OMSetDepthStencilState(DepthStencilState, 0);
}


//=============================================

void URenderer::createConstantBuffer()
{
	D3D11_BUFFER_DESC constantbufferdesc = {};
	constantbufferdesc.ByteWidth = sizeof(FConstants) + 0xf & 0xfffffff0; // ensure constant buffer size is multiple of 16 bytes
	constantbufferdesc.Usage = D3D11_USAGE_DYNAMIC; // will be updated from CPU every frame
	constantbufferdesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	constantbufferdesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

	Device->CreateBuffer(&constantbufferdesc, nullptr, &ConstantBuffer);
}

void URenderer::releaseConstantBuffer()
{
	if (ConstantBuffer)
	{
		ConstantBuffer->Release();
		ConstantBuffer = nullptr;
	}
}

void URenderer::createDepthStencilBuffer(UINT width, UINT height)
{
	D3D11_TEXTURE2D_DESC desc = {};

	desc.Width = width;   // 백버퍼와 크기가 정확히 같아야 함
	desc.Height = height;

	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;  // 깊이 24비트 + 스텐실 8비트
	desc.SampleDesc.Count = 1;                    // 스왑체인의 SampleDesc와 반드시 동일
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;    // 이 플래그가 없으면 DSV 생성 실패

	Device->CreateTexture2D(&desc, nullptr, &DepthStencilBuffer);

	D3D11_DEPTH_STENCIL_VIEW_DESC dsvdesc = {};
	dsvdesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvdesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;

	Device->CreateDepthStencilView(DepthStencilBuffer, &dsvdesc, &DepthStencilView);
}

void URenderer::createDepthStencilState()
{
	D3D11_DEPTH_STENCIL_DESC desc = {};
	desc.DepthEnable = TRUE;							 // 깊이 테스트 켜기
	desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;    // 통과한 픽셀의 z를 기록
	desc.DepthFunc = D3D11_COMPARISON_LESS;				 // 더 가까우면(작으면) 통과
	desc.StencilEnable = FALSE;

	Device->CreateDepthStencilState(&desc, &DepthStencilState);
}

void URenderer::createStencilMarkState()
{
	D3D11_DEPTH_STENCIL_DESC desc = {};
	// 아웃라인 패스가 깊이를 무시하므로 마킹도 깊이를 무시해야 짝이 맞는다.
	// 가려진 픽셀까지 전부 마킹해야 실루엣 내부가 비지 않는다.
	desc.DepthEnable = FALSE;
	desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;  // 깊이는 건드리지 않는다
	desc.DepthFunc = D3D11_COMPARISON_ALWAYS;

	desc.StencilEnable = TRUE;							// 스텐실 사용
	desc.StencilReadMask = 0xFF;
	desc.StencilWriteMask = 0xFF;

	// 실루엣에 덮이는 모든 픽셀에 StencilRef를 기록
	desc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
	desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_REPLACE;
	desc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	desc.BackFace = desc.FrontFace;

	Device->CreateDepthStencilState(&desc, &StencilMarkState);
}

void URenderer::createStencilOutlineState()
{
	D3D11_DEPTH_STENCIL_DESC desc = {};
	desc.DepthEnable = FALSE;							// 항상 위에 그린다
	desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;

	desc.StencilEnable = TRUE;
	desc.StencilReadMask = 0xFF;
	desc.StencilWriteMask = 0x00;						// 읽기만, 쓰지 않는다

	// 마킹된 곳(=원본 실루엣)은 통과 못 함 -> 바깥 테두리만 남는다
	desc.FrontFace.StencilFunc = D3D11_COMPARISON_NOT_EQUAL;
	desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	desc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	desc.BackFace = desc.FrontFace;

	Device->CreateDepthStencilState(&desc, &StencilOutlineState);
}

// 렌더타겟에 색을 전혀 쓰지 않는 상태. 스텐실 마킹 전용 패스에 쓴다
void URenderer::createNoColorWriteBlendState()
{
	D3D11_BLEND_DESC desc = {};
	desc.RenderTarget[0].BlendEnable = FALSE;
	desc.RenderTarget[0].RenderTargetWriteMask = 0;

	Device->CreateBlendState(&desc, &NoColorWriteBlendState);
}

void URenderer::releaseBlendState()
{
	if (NoColorWriteBlendState) { NoColorWriteBlendState->Release(); NoColorWriteBlendState = nullptr; }
	if (FontBlendState) { FontBlendState->Release(); FontBlendState = nullptr; }
	if (ParticleBlendState) { ParticleBlendState->Release(); ParticleBlendState = nullptr; }
}

void URenderer::releaseDepthStencilBuffer()
{
	if (DepthStencilView) { DepthStencilView->Release();   DepthStencilView = nullptr; }
	if (DepthStencilBuffer) { DepthStencilBuffer->Release(); DepthStencilBuffer = nullptr; }
}

void URenderer::releaseDepthStencilState()
{
	if (DepthStencilState) { DepthStencilState->Release();  DepthStencilState = nullptr; }
	if (StencilMarkState) { StencilMarkState->Release();  StencilMarkState = nullptr; }
	if (StencilOutlineState) { StencilOutlineState->Release();  StencilOutlineState = nullptr; }
}

void URenderer::UpdateConstant(FMatrix world, FMatrix viewProjection, FVector4 tint)
{
	if (ConstantBuffer)
	{
		D3D11_MAPPED_SUBRESOURCE constantbufferMSR;

		DeviceContext->Map(ConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &constantbufferMSR); // update constant buffer every frame
		FConstants* constants = (FConstants*)constantbufferMSR.pData;
		{
			constants->World = world;
			constants->ViewProjection = viewProjection;
			constants->Tint = tint;
		}
		DeviceContext->Unmap(ConstantBuffer, 0);
	}
}

void URenderer::UpdateFontBuffer(const TArray<FVertexTextured>& vertices, const TArray<uint32>& indices, uint32 numCharacter)
{
	const uint32 numVertices = vertices.Num();

	// Increase Buffer Size if Needed
	// TODO: Move to a separate function to handle buffer resizing
	if (!FontTextureBuffer || numVertices > mTextVertexCapacity)
	{
		const uint32 newCapacity = FMath::Max(numVertices, static_cast<uint32>(mTextVertexCapacity * 2));

		D3D11_BUFFER_DESC bufferDesc = {};
		bufferDesc.ByteWidth = newCapacity * sizeof(FVertexTextured);
		bufferDesc.Usage = D3D11_USAGE_DYNAMIC; // 동적 설정
		bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE; // 동적으로 CPU가 쓰기 가능

		ID3D11Buffer* newBuffer = nullptr;
		Device->CreateBuffer(&bufferDesc, nullptr, &newBuffer);
		UE_LOG(Log, Render, "Font buffer CREATE: capacity %u -> %u",
			mTextVertexCapacity, newCapacity);

		// 새 버퍼 생성에 성공한 뒤 기존 버퍼를 교체
		//ReleaseFontAtlasQuad();
		if (FontTextureBuffer)
		{
			FontTextureBuffer->Release();
		}
		FontTextureBuffer = newBuffer;
		mTextVertexCapacity = newCapacity;
	}

	// Map the vertex buffer and copy the vertex data
	D3D11_MAPPED_SUBRESOURCE mappedResource = {};
	DeviceContext->Map(FontTextureBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	memcpy(mappedResource.pData, vertices.GetData(), numVertices * sizeof(FVertexTextured));
	DeviceContext->Unmap(FontTextureBuffer, 0);



	const uint32 numIndices = indices.Num();
	ensureFontIndexBuffer(numIndices / 4);
}

void URenderer::UpdateParticleBuffer(const TArray<FVertexTextured>& vertices, const TArray<uint32>& indices)
{
	assert(ParticleVertexBuffer && ParticleBlendState);

	const uint32 numVertices = vertices.Num();

	// Map the vertex buffer and copy the vertex data
	D3D11_MAPPED_SUBRESOURCE mappedResource = {};
	DeviceContext->Map(ParticleVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	memcpy(mappedResource.pData, vertices.GetData(), numVertices * sizeof(FVertexTextured));
	DeviceContext->Unmap(ParticleVertexBuffer, 0);

	// Map the index buffer and copy the index data
	D3D11_MAPPED_SUBRESOURCE mappedIndexResource = {};
	DeviceContext->Map(ParticleIndexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedIndexResource);
	memcpy(mappedIndexResource.pData, indices.GetData(), indices.Num() * sizeof(uint32));
	DeviceContext->Unmap(ParticleIndexBuffer, 0);
}

void URenderer::OnResize(UINT width, UINT height, float viewportWidth, float viewportHeight)
{
	if (!SwapChain || width == 0 || height == 0) return;
	if (ViewportInfo.Width == viewportWidth && ViewportInfo.Height == viewportHeight) return;

	//해상도에 의존하는 프레임 버퍼와 뎁스 스텐실 버퍼를 재생성한다.
	DeviceContext->OMSetRenderTargets(0, nullptr, nullptr);
	releaseFrameBuffer();
	releaseDepthStencilBuffer();

	HRESULT hr = SwapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);
	if (FAILED(hr)) return;

	DXGI_SWAP_CHAIN_DESC desc;
	SwapChain->GetDesc(&desc);

	ViewportInfo = { viewportWidth, 0.0f, static_cast<float>(width) - viewportWidth, viewportHeight, 0.0f, 1.0f };

	//상태는 이전에 생성한 걸 그대로 재사용
	createFrameBuffer();
	createDepthStencilBuffer(width, height);
}

void URenderer::ClearDepth()
{
	DeviceContext->ClearDepthStencilView(DepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
}

ID3D11Buffer* URenderer::CreatePrimitiveIndexBuffer(const UINT* indices, UINT indexCount)
{
	if (!indices || indexCount == 0)
		return nullptr;

	D3D11_BUFFER_DESC desc = {};
	desc.Usage = D3D11_USAGE_IMMUTABLE;
	desc.ByteWidth = indexCount * sizeof(unsigned int);
	desc.BindFlags = D3D11_BIND_INDEX_BUFFER; // 인데스 버퍼라고 명시

	D3D11_SUBRESOURCE_DATA data = {};
	data.pSysMem = indices;

	ID3D11Buffer* buffer = nullptr;
	HRESULT hr = Device->CreateBuffer(&desc, &data, &buffer);

	return SUCCEEDED(hr) ? buffer : nullptr;
}

bool URenderer::ensureFontIndexBuffer(UINT fontCpunt)
{
	if (fontCpunt == 0)
		return true;

	if (FontIndexBuffer && fontCpunt <= mTextIndexCapacity)
		return true;

	const UINT newCapacity = (std::max)(fontCpunt, mTextIndexCapacity * 2);

	std::vector<UINT> indices;
	indices.reserve(static_cast<size_t>(newCapacity) * 6);

	for (UINT i = 0; i < newCapacity; ++i)
	{
		const UINT base = i * 4; // 다음 문자의 시작 정점 번호로 이동

		indices.push_back(base);
		indices.push_back(base + 1);
		indices.push_back(base + 2);

		indices.push_back(base + 2);
		indices.push_back(base + 3);
		indices.push_back(base + 0);
	}

/*	D3D11_BUFFER_DESC desc = {};
	desc.ByteWidth = static_cast<UINT>(indices.size() * sizeof(UINT));
	desc.Usage = D3D11_USAGE_IMMUTABLE; // 
	desc.BindFlags = D3D11_BIND_INDEX_BUFFER; // 인덱스 버퍼라고 명시

	D3D11_SUBRESOURCE_DATA data = {};
	data.pSysMem = indices.data();

	ID3D11Buffer* newBuffer = nullptr;
	HRESULT hr = Device->CreateBuffer(&desc, &data, &newBuffer);*/

	ID3D11Buffer* newBuffer = CreatePrimitiveIndexBuffer(indices.data(), static_cast<UINT>(indices.size())
	);

	if (!newBuffer)
		return false;

	if (FontIndexBuffer)
		FontIndexBuffer->Release();

	FontIndexBuffer = newBuffer;
	mTextIndexCapacity = newCapacity;
	return true;
}

void URenderer::releaseFontTexture()
{
	// 매번 해제하면 버퍼를 재사용 불가능
	//ReleaseFontAtlasQuad();

	if (FontSamplerState)
	{
		FontSamplerState->Release();
		FontSamplerState = nullptr;
	}

	if (FontBlendState)
	{
		FontBlendState->Release();
		FontBlendState = nullptr;
	}
}

void URenderer::ReleasePrimitiveTextureResources(
	ID3D11ShaderResourceView* textureSRV, ID3D11SamplerState* samplerState)
{
	if (textureSRV)
	{
		textureSRV->Release();
		textureSRV = nullptr;
	}

	if (samplerState)
	{
		samplerState->Release();
		samplerState = nullptr;
	}
}


