#include "Renderer.h"

#include <fstream>
#include <filesystem>
#include <vector>

//#include "WICTextureLoader.h"
#include <directxtk/DDSTextureLoader.h>
#include "Console.h"

#pragma comment(lib, "DirectXTK.lib")
#pragma comment(lib, "dxguid.lib")
//#pragma comment(lib, "windowscodecs.lib")
//#pragma comment(lib, "ole32.lib")

void URenderer::Create(HWND hWindow)
{
	CreateDeviceAndSwapChain(hWindow);
	CreateFrameBuffer();
	//CreateDepthStencilBuffer();

	/*if (!CreateTestTexture())
	{
		OutputDebugStringA("CreateTestTexture failed.\n");
	}

	if (!CreateTestQuad())
    {
        OutputDebugStringA("CreateTestTexture failed.\n");
    }*/

	// Device가 만들어진 이후에 호출해야 한다.
	/*if (!CreateFontAtlasTexture())
	{
		MessageBox(hWindow,
			L"파일을 불러오지 못했습니다.",
			L"Font atlas load error",
			MB_OK | MB_ICONERROR
		);
	}*/

	/*if (!CreateTestQuad())
	{
		MessageBox(
			hWindow,
			L"폰트 Quad 생성에 실패했습니다.",
			L"Font quad error",
			MB_OK | MB_ICONERROR
		);
	}*/

	// 테스트용 문자열
	/*std::string text = "Hello!";

	if (!CreateFontAtlasQuad(&text))
	{
		MessageBox(
			hWindow,
			L"폰트 Quad 생성에 실패했습니다.",
			L"Font quad error",
			MB_OK | MB_ICONERROR
		);
	}*/

	if (!CreateFontAtlasTexture() ||
		!CreateFontShader() ||
		!CreateFontSamplerState() ||
		!CreateFontBlendState())
	{
		MessageBox(
			hWindow,
			L"폰트 공통 자원 생성에 실패했습니다.",
			L"Font initialization error",
			MB_OK | MB_ICONERROR
		);

		ReleaseFontTexture();
		ReleaseFontAtlasTexture();
	}
	else
	{
		// 테스트용 문자열
		std::string text = "Hello Jungle!";

		if (!CreateFontAtlasQuad(&text))
		{
			MessageBox(
				hWindow,
				L"폰트 vertex 버퍼 생성에 실패했습니다.",
				L"Font quad error",
				MB_OK | MB_ICONERROR
			);
		}
	}


	CreateDepthStencilState();
	CreateStencilMarkState();
	CreateStencilOutlineState();
	CreateNoColorWriteBlendState();
	CreateRasterizerState();

	// 일반 Primitive용 텍스처 리소스 생성
	// 경로는 실제 보유한 DDS 파일 경로로 변경

	// CubeTextureSample.dds / Dice.dds
/*	if (!CreatePrimitiveTextureResources())
	{
		MessageBox(
			hWindow,
			L"Primitive 텍스처 리소스 생성에 실패했습니다.",
			L"Primitive texture initialization error",
			MB_OK | MB_ICONERROR);
	}
	// 바로 여기에 추가
	if (!LoadTexture(L"CubeTextureSample.dds", &CubeTextureSRV))
	{
		OutputDebugStringA("Cube texture load failed.\n");
	}

	if (!LoadTexture(L"EarthTexture.dds", &SphereTextureSRV))
	{
		OutputDebugStringA("Sphere texture load failed.\n");
	}
	//if (!CreatePrimitiveTextureResources(L"Dice.dds"))
	//{
	//	MessageBox(
	//		hWindow,
	//		L"Primitive 텍스처 리소스 생성에 실패했습니다.",
	//		L"Primitive texture initialization error",
	//		MB_OK | MB_ICONERROR);
	//}*/
}

void URenderer::CreateDeviceAndSwapChain(HWND hWindow)
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

void URenderer::ReleaseDeviceAndSwapChain()
{

	if (FontIndexBuffer)
	{
		FontIndexBuffer->Release();
		FontIndexBuffer = nullptr;

		mTextIndexCount = 0;
		mTextIndexCapacity = 0;
	}

	
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

void URenderer::CreateFrameBuffer()
{
	SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&FrameBuffer);

	D3D11_RENDER_TARGET_VIEW_DESC framebufferRTVdesc = {};
	framebufferRTVdesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
	framebufferRTVdesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

	Device->CreateRenderTargetView(FrameBuffer, &framebufferRTVdesc, &FrameBufferRTV);
}

void URenderer::ReleaseFrameBuffer()
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

bool URenderer::CreateFontAtlasTexture()
{
	if (Device == nullptr)
	{
		return false;
	}

	// 함수가 다시 호출되는 경우 기존 텍스처 해제
	
	ReleaseFontAtlasTexture();

	// 추후 동적으로 텍스쳐 로드하자 
	HRESULT hr = DirectX::CreateDDSTextureFromFile(
		Device,
		L"EnglishBigFontAtlas.dds",
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

void URenderer::ReleaseFontAtlasTexture()
{
	if (FontAtlasShaderResoruceView != nullptr)
	{
		FontAtlasShaderResoruceView->Release();
		FontAtlasShaderResoruceView = nullptr;
	}
}

bool URenderer::CreateFontSamplerState()
{
	if (!Device)
		return false;

	if (TextureSamplerState)
	{
		TextureSamplerState->Release();
		TextureSamplerState = nullptr;
	}

	D3D11_SAMPLER_DESC desc = {};
	desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	desc.MaxAnisotropy = 1;
	desc.MaxLOD = D3D11_FLOAT32_MAX;

	return SUCCEEDED(Device->CreateSamplerState(&desc, &TextureSamplerState)
	);
}

bool URenderer::CreateFontBlendState()
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

void URenderer::ReleaseFontAtlasQuad()
{
	if (FontTextureBuffer)
	{
		FontTextureBuffer->Release();
		FontTextureBuffer = nullptr;
	}

	mTextVertexCount = 0;
	mTextVertexCapacity = 0;
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
void URenderer::CreateLineVertexBuffer(uint32 maxVertices)
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

void URenderer::ReleaseLineVertexBuffer()
{
	if (LineVertexBuffer)
	{
		LineVertexBuffer->Release();
		LineVertexBuffer = nullptr;
	}

	LineVertexCapacity = 0;
}

void URenderer::CreateLineIndexBuffer(uint32 maxIndices)
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

void URenderer::ReleaseLineIndexBuffer()
{
	if (LineIndexBuffer)
	{
		LineIndexBuffer->Release();
		LineIndexBuffer = nullptr;
	}

	LineIndexCapacity = 0;
}

void URenderer::CreateRasterizerState()
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

void URenderer::ReleaseRasterizerState()
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
	ReleaseRasterizerState();

	DeviceContext->OMSetRenderTargets(0, nullptr, nullptr);

	ReleaseDepthStencilBuffer();
	ReleaseDepthStencilState();
	ReleaseBlendState();
	ReleaseFrameBuffer();

	// 테스트

	ReleaseFontAtlasTexture();
	ReleaseFontTexture();
	//ReleaseTestTexture();
	ReleaseDeviceAndSwapChain();
}

void URenderer::SwapBuffer()
{
	SwapChain->Present(1, 0);
}

void URenderer::CreateShader()
{
	ID3DBlob* vertexshaderCSO;
	ID3DBlob* pixelshaderCSO;
	ID3DBlob* LinevertexshaderCSO;
	ID3DBlob* LinepixelshaderCSO;
	ID3DBlob* primitiveTextureVertexShaderCSO;
	ID3DBlob* primitiveTexturePixelShaderCSO;


	D3DCompileFromFile(L"ShaderW0.hlsl", nullptr, nullptr, "mainVS", "vs_5_0", 0, 0, &vertexshaderCSO, nullptr);

	Device->CreateVertexShader(vertexshaderCSO->GetBufferPointer(), vertexshaderCSO->GetBufferSize(), nullptr, &SimpleVertexShader);

	D3DCompileFromFile(L"ShaderW0.hlsl", nullptr, nullptr, "mainPS", "ps_5_0", 0, 0, &pixelshaderCSO, nullptr);

	Device->CreatePixelShader(pixelshaderCSO->GetBufferPointer(), pixelshaderCSO->GetBufferSize(), nullptr, &SimplePixelShader);

	D3DCompileFromFile(L"ShaderLine.hlsl", nullptr, nullptr, "mainVS", "vs_5_0", 0, 0, &LinevertexshaderCSO, nullptr);

	Device->CreateVertexShader(LinevertexshaderCSO->GetBufferPointer(), LinevertexshaderCSO->GetBufferSize(), nullptr, &LineSimpleVertexShader);

	D3DCompileFromFile(L"ShaderLine.hlsl", nullptr, nullptr, "mainPS", "ps_5_0", 0, 0, &LinepixelshaderCSO, nullptr);

	Device->CreatePixelShader(LinepixelshaderCSO->GetBufferPointer(), LinepixelshaderCSO->GetBufferSize(), nullptr, &LineSimplePixelShader);

	D3DCompileFromFile(L"ShaderTexture.hlsl",nullptr,	nullptr,"mainVS","vs_5_0",0,0,	&primitiveTextureVertexShaderCSO, nullptr);

	Device->CreateVertexShader(primitiveTextureVertexShaderCSO->GetBufferPointer(), primitiveTextureVertexShaderCSO->GetBufferSize(), nullptr,	&PrimitiveTextureVertexShader);

	D3DCompileFromFile(L"ShaderTexture.hlsl", nullptr,	nullptr,"mainPS", "ps_5_0",	0,	0,	&primitiveTexturePixelShaderCSO, nullptr);

	Device->CreatePixelShader(primitiveTexturePixelShaderCSO->GetBufferPointer(), primitiveTexturePixelShaderCSO->GetBufferSize(), nullptr, &PrimitiveTexturePixelShader);


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

	Device->CreateInputLayout(layout, ARRAYSIZE(layout), vertexshaderCSO->GetBufferPointer(), vertexshaderCSO->GetBufferSize(), &SimpleInputLayout);
	Device->CreateInputLayout(Linelayout, ARRAYSIZE(Linelayout), LinevertexshaderCSO->GetBufferPointer(), LinevertexshaderCSO->GetBufferSize(), &LineSimpleInputLayout);
	Device->CreateInputLayout(primitiveTextureLayout,ARRAYSIZE(primitiveTextureLayout), primitiveTextureVertexShaderCSO->GetBufferPointer(), primitiveTextureVertexShaderCSO->GetBufferSize(), &PrimitiveTextureLayout);

	StrideSimple = sizeof(FVertexSimple);
	StrideTextured = sizeof(FVertexTextured);

	vertexshaderCSO->Release();
	pixelshaderCSO->Release();
	LinevertexshaderCSO->Release();
	LinepixelshaderCSO->Release();
	primitiveTextureVertexShaderCSO->Release();
	primitiveTexturePixelShaderCSO->Release();


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

void URenderer::ReleaseFontShader()
{
	if (TextureVertexShader)
	{
		TextureVertexShader->Release();
		TextureVertexShader = nullptr;
	}

	if (TexturePixelShader)
	{
		TexturePixelShader->Release();
		TexturePixelShader = nullptr;
	}

	if (TextureInputLayout)
	{
		TextureInputLayout->Release();
		TextureInputLayout = nullptr;
	}
}

bool URenderer::CreateFontShader()
{
	if (!Device)
		return false;

	ReleaseFontShader();

	ID3DBlob* vsCode = nullptr;
	ID3DBlob* psCode = nullptr;

	bool success = false;

	do
	{
		HRESULT hr = D3DCompileFromFile(
			L"ShaderFont.hlsl", nullptr, nullptr,
			"mainVS", "vs_5_0", 0, 0,
			&vsCode, nullptr
		);

		if (FAILED(hr))
			break;

		hr = D3DCompileFromFile(
			L"ShaderFont.hlsl", nullptr, nullptr,
			"mainPS", "ps_5_0", 0, 0,
			&psCode, nullptr
		);

		if (FAILED(hr))
			break;

		hr = Device->CreateVertexShader(
			vsCode->GetBufferPointer(),
			vsCode->GetBufferSize(),
			nullptr,
			&TextureVertexShader
		);

		if (FAILED(hr))
			break;

		hr = Device->CreatePixelShader(
			psCode->GetBufferPointer(),
			psCode->GetBufferSize(),
			nullptr,
			&TexturePixelShader
		);

		if (FAILED(hr))
			break;

		const D3D11_INPUT_ELEMENT_DESC layout[] =
		{
			{
				"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,
				0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0
			},
			{
				"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,
				0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0
			}
		};

		hr = Device->CreateInputLayout(
			layout,
			ARRAYSIZE(layout),
			vsCode->GetBufferPointer(),
			vsCode->GetBufferSize(),
			&TextureInputLayout
		);

		if (FAILED(hr))
			break;

		success = true;
	} while (false);

	if (vsCode)
		vsCode->Release();

	if (psCode)
		psCode->Release();

	// 중간에 실패한 경우, 이미 생성한 자원도 정리
	if (!success)
		ReleaseFontShader();

	return success;
}


void URenderer::ReleaseShader()
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
	if (TextureInputLayout)
	{
		TextureInputLayout->Release();
		TextureInputLayout = nullptr;
	}

	if (TexturePixelShader)
	{
		TexturePixelShader->Release();
		TexturePixelShader = nullptr;
	}

	if (TextureVertexShader)
	{
		TextureVertexShader->Release();
		TextureVertexShader = nullptr;
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

void URenderer::Prepare(bool bWireFrame)
{
	DeviceContext->ClearRenderTargetView(FrameBufferRTV, ClearColor);

	//매 프레임 깊이 버퍼를 1.0(가장 먼 값)으로 초기화
	DeviceContext->ClearDepthStencilView(DepthStencilView,
		D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	DeviceContext->RSSetViewports(1, &ViewportInfo);

	DeviceContext->RSSetState(RasterizerState[bWireFrame ? 1 : 0]);

	//세 번째 인자에 nullptr 대신 DSV를 넘긴다
	DeviceContext->OMSetRenderTargets(1, &FrameBufferRTV, DepthStencilView);
	//깊이 테스트 규칙 적용
	DeviceContext->OMSetDepthStencilState(DepthStencilState, 0);
	DeviceContext->OMSetBlendState(nullptr, nullptr, 0xffffffff);
}
void URenderer::RSUpdateState()
{
	DeviceContext->RSSetState(RasterizerState[0]);
}

void URenderer::PrepareSimpleShader()
{
	DeviceContext->VSSetShader(SimpleVertexShader, nullptr, 0);
	DeviceContext->PSSetShader(SimplePixelShader, nullptr, 0);
	DeviceContext->IASetInputLayout(SimpleInputLayout);

	if (ConstantBuffer)
	{
		DeviceContext->VSSetConstantBuffers(0, 1, &ConstantBuffer);
	}
}

void URenderer::PrepareTextureShader()
{
	DeviceContext->VSSetShader(PrimitiveTextureVertexShader, nullptr, 0);
	DeviceContext->PSSetShader(PrimitiveTexturePixelShader, nullptr, 0);
	DeviceContext->IASetInputLayout(PrimitiveTextureLayout);

	if (ConstantBuffer)
	{
		DeviceContext->VSSetConstantBuffers(0, 1, &ConstantBuffer);
	}
}

void URenderer::PrepareLineShader()
{
	DeviceContext->VSSetShader(LineSimpleVertexShader, nullptr, 0);
	DeviceContext->PSSetShader(LineSimplePixelShader, nullptr, 0);
	DeviceContext->IASetInputLayout(LineSimpleInputLayout);

	if (ConstantBuffer)
	{
		DeviceContext->VSSetConstantBuffers(0, 1, &ConstantBuffer);
	}
}

void URenderer::RenderSimplePrimitive(ID3D11Buffer* pBuffer, UINT numVertices)
{
	PrepareSimpleShader();
	UINT offset = 0;
	// Bind the vertex buffer
	DeviceContext->IASetVertexBuffers(0, 1, &pBuffer, &StrideSimple, &offset);
	DeviceContext->Draw(numVertices, 0);
}

void URenderer::RenderTexturePrimitive(ID3D11Buffer* pBuffer, UINT numVertices,
	ID3D11ShaderResourceView* texture, ID3D11SamplerState* samplerState)
{
	UINT offset = 0;
	// Bind the vertex buffer
	DeviceContext->IASetVertexBuffers(0, 1, &pBuffer, &StrideTextured, &offset);

	// Bind the texture resource
	DeviceContext->PSSetShaderResources(0, 1, &texture);
	DeviceContext->PSSetSamplers(0, 1, &samplerState);


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
	DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	PrepareLineShader();
	DeviceContext->DrawIndexed(numindices, 0, 0);

	PrepareSimpleShader();
	DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
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

void URenderer::CreateConstantBuffer()
{
	D3D11_BUFFER_DESC constantbufferdesc = {};
	constantbufferdesc.ByteWidth = sizeof(FConstants) + 0xf & 0xfffffff0; // ensure constant buffer size is multiple of 16 bytes
	constantbufferdesc.Usage = D3D11_USAGE_DYNAMIC; // will be updated from CPU every frame
	constantbufferdesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	constantbufferdesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

	Device->CreateBuffer(&constantbufferdesc, nullptr, &ConstantBuffer);
}

void URenderer::ReleaseConstantBuffer()
{
	if (ConstantBuffer)
	{
		ConstantBuffer->Release();
		ConstantBuffer = nullptr;
	}
}

void URenderer::CreateDepthStencilBuffer(UINT width, UINT height)
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

void URenderer::CreateDepthStencilState()
{
	D3D11_DEPTH_STENCIL_DESC desc = {};
	desc.DepthEnable = TRUE;							 // 깊이 테스트 켜기
	desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;    // 통과한 픽셀의 z를 기록
	desc.DepthFunc = D3D11_COMPARISON_LESS;				 // 더 가까우면(작으면) 통과
	desc.StencilEnable = FALSE;

	Device->CreateDepthStencilState(&desc, &DepthStencilState);
}

void URenderer::CreateStencilMarkState()
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

void URenderer::CreateStencilOutlineState()
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
void URenderer::CreateNoColorWriteBlendState()
{
	D3D11_BLEND_DESC desc = {};
	desc.RenderTarget[0].BlendEnable = FALSE;
	desc.RenderTarget[0].RenderTargetWriteMask = 0;

	Device->CreateBlendState(&desc, &NoColorWriteBlendState);
}

void URenderer::ReleaseBlendState()
{
	if (NoColorWriteBlendState) { NoColorWriteBlendState->Release(); NoColorWriteBlendState = nullptr; }
}

void URenderer::ReleaseDepthStencilBuffer()
{
	if (DepthStencilView) { DepthStencilView->Release();   DepthStencilView = nullptr; }
	if (DepthStencilBuffer) { DepthStencilBuffer->Release(); DepthStencilBuffer = nullptr; }
}

void URenderer::ReleaseDepthStencilState()
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

void URenderer::OnResize(UINT width, UINT height, float viewportWidth, float viewportHeight)
{
	if (!SwapChain || width == 0 || height == 0) return;
	if (ViewportInfo.Width == viewportWidth && ViewportInfo.Height == viewportHeight) return;

	//해상도에 의존하는 프레임 버퍼와 뎁스 스텐실 버퍼를 재생성한다.
	DeviceContext->OMSetRenderTargets(0, nullptr, nullptr);
	ReleaseFrameBuffer();
	ReleaseDepthStencilBuffer();

	HRESULT hr = SwapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);
	if (FAILED(hr)) return;

	DXGI_SWAP_CHAIN_DESC desc;
	SwapChain->GetDesc(&desc);

	ViewportInfo = { viewportWidth, 0.0f, static_cast<float>(width) - viewportWidth, viewportHeight, 0.0f, 1.0f };

	//상태는 이전에 생성한 걸 그대로 재사용
	CreateFrameBuffer();
	CreateDepthStencilBuffer(width, height);
}

void URenderer::ClearDepth()
{
	DeviceContext->ClearDepthStencilView(DepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
}


// 테스트용 쿼드 출력 나중에 지울 예정

bool URenderer::CreateTestQuad()
{
	if (!Device)
		return false;

	ReleaseFontTexture();

	mTextVertexCount = 0;

	std::vector<FVertexTextured> verticesList;
	const std::string  text = "Hello Jungle!"; // 예시 문자열

	// 화면에 표시할 문자 한 개의 크기
	const float charWidth = 0.1f;
	const float charHeight = 0.2f;

	//글자 사이 간격
	const float charAdvance = charWidth * 0.7f;

	const float textWidth =
		text.empty() ? 0.0f : static_cast<float>(text.size() - 1) * charAdvance + charWidth;

	// 문자 가운데 정렬
	const float startX = -textWidth * 0.5f;
	const float startY = charHeight * 0.5f;


	// dds의 배치
	const int columns = 16;
	const int rows = 16;

	// 한 칸의 크기
	const float cellWidth = 1.0f / columns;
	const float cellHeight = 1.0f / rows;

	verticesList.reserve(text.size() * 6);
	
	const int firstCharacter = 0;


	for (size_t i = 0; i < text.size(); ++i)
	{
		const unsigned char character = static_cast<unsigned char>(text[i]);

		int charCode = (int)character; // 아스키 코드 값


		const int fontAtlasIndex = charCode - firstCharacter;


		const float u0 = (fontAtlasIndex % columns) * cellWidth;
		const float v0 = (fontAtlasIndex / columns) * cellHeight;
		const float u1 = u0 + cellWidth;
		const float v1 = v0 + cellHeight;

		// 화면에서 해당 문자의 사각형 위치
		const float left = startX + static_cast<float>(i) * charAdvance;
		const float right = left + charWidth;
		const float top = startY;
		const float bottom = top - charHeight;

		// 삼각형
		
		// Billboard local plane: X = 0, horizontal = Y, vertical = Z.
		verticesList.push_back({ 0.0f, left, top, u0, v0 });
		verticesList.push_back({ 0.0f, right, top, u1, v0 });
		verticesList.push_back({ 0.0f, left, bottom, u0, v1 });

		verticesList.push_back({ 0.0f, right, bottom, u1, v1 });
		verticesList.push_back({ 0.0f, left, bottom, u0, v1 });
		verticesList.push_back({ 0.0f, right, top, u1, v0 });


	}
	
	mTextVertexCount = static_cast<UINT>(verticesList.size());

	// 문자열이 비어있다
	if (mTextVertexCount == 0)
		return false;

	ID3DBlob* vsCode = nullptr;
	ID3DBlob* psCode = nullptr;

	bool success = false;

	do
	{
		// HLSL 컴파일
		HRESULT hr = D3DCompileFromFile(L"ShaderFont.hlsl",	nullptr, nullptr, "mainVS", "vs_5_0", 0, 0, &vsCode, nullptr);

		if (FAILED(hr))
			break;

		hr = D3DCompileFromFile(L"ShaderFont.hlsl",	nullptr, nullptr, "mainPS", "ps_5_0", 0, 0, &psCode, nullptr);

		if (FAILED(hr))
			break;

		// GPU 셰이더 생성
		hr = Device->CreateVertexShader(vsCode->GetBufferPointer(),	vsCode->GetBufferSize(), nullptr, &TextureVertexShader);

		if (FAILED(hr))
			break;

		hr = Device->CreatePixelShader(psCode->GetBufferPointer(),	psCode->GetBufferSize(), nullptr, &TexturePixelShader);

		if (FAILED(hr))
			break;

		// 정점 메모리 구조를 셰이더 입력과 연결
		const D3D11_INPUT_ELEMENT_DESC layout[] =
		{
			{
				"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,
				0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0
			},
			{
				"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,
				0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0
			},
		};

		hr = Device->CreateInputLayout(layout,	ARRAYSIZE(layout),	vsCode->GetBufferPointer(),	vsCode->GetBufferSize(),
			&TextureInputLayout);

		if (FAILED(hr))
			break;

		// 문자별 정점과 UV를 GPU에 저장
		D3D11_BUFFER_DESC bufferDesc = {};
		bufferDesc.ByteWidth = mTextVertexCount * sizeof(FVertexTextured);
		bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

		D3D11_SUBRESOURCE_DATA initialData = {};
		initialData.pSysMem = verticesList.data();

		hr = Device->CreateBuffer(&bufferDesc,	&initialData, &FontTextureBuffer);

		if (FAILED(hr))
			break;

		
		D3D11_SAMPLER_DESC samplerDesc = {};
		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		samplerDesc.MaxAnisotropy = 1;
		samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

		hr = Device->CreateSamplerState(&samplerDesc,	&TextureSamplerState);

		if (FAILED(hr))
			break;

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

		hr = Device->CreateBlendState(&desc, &FontBlendState);
		if (FAILED(hr))
			break;

		success = true;
	} while (false);

	if (vsCode) vsCode->Release();
	if (psCode) psCode->Release();

	if (!success)
		ReleaseFontTexture();

	return success;
}

// 폰트 1개가 나올 Quad를 그리는 함수
bool URenderer::CreateFontAtlasQuad(std::string* Text)
{
	if (!Device || !Text || !DeviceContext)
		return false;

	//ReleaseFontAtlasQuad();

	std::vector<FVertexTextured> verticesList;

	mTextVertexCount = 0;

	// 인덱스는 Quad를 그릴때 마다 초기화
	mTextIndexCount = 0;
	
	

	// 화면에 표시할 문자 한 개의 크기
	const float charWidth = 0.1f;
	const float charHeight = 0.2f;

	//글자 사이 간격
	const float charAdvance = charWidth * 0.7f;

	const float textWidth =
		Text->empty() ? 0.0f : static_cast<float>(Text->size() - 1) * charAdvance + charWidth;

	// 문자 가운데 정렬
	const float startX = -textWidth * 0.5f;
	const float startY = charHeight * 0.5f;


	// dds의 배치
	const int columns = 16;
	const int rows = 16;

	// 한 칸의 크기
	const float cellWidth = 1.0f / columns;
	const float cellHeight = 1.0f / rows;

	// verticesList.reserve(Text->size() * 6);

	// index buffer 사용시
	verticesList.reserve(Text->size() * 4);

	const int firstCharacter = 0;


	for (size_t i = 0; i < Text->size(); ++i)
	{
		const unsigned char character = static_cast<unsigned char>((*Text)[i]);

		int charCode = (int)character; // 아스키 코드 값


		const int fontAtlasIndex = charCode - firstCharacter;


		const float u0 = (fontAtlasIndex % columns) * cellWidth;
		const float v0 = (fontAtlasIndex / columns) * cellHeight;
		const float u1 = u0 + cellWidth;
		const float v1 = v0 + cellHeight;

		// 화면에서 해당 문자의 사각형 위치
		const float left = startX + static_cast<float>(i) * charAdvance;
		const float right = left + charWidth;
		const float top = startY;
		const float bottom = top - charHeight;

		// 삼각형

		// 기존 방식
		// Billboard local plane: X = 0, horizontal = Y, vertical = Z.
		/*verticesList.push_back({ 0.0f, left, top, u0, v0 });
		verticesList.push_back({ 0.0f, right, top, u1, v0 });
		verticesList.push_back({ 0.0f, left, bottom, u0, v1 });

		verticesList.push_back({ 0.0f, right, bottom, u1, v1 });
		verticesList.push_back({ 0.0f, left, bottom, u0, v1 });
		verticesList.push_back({ 0.0f, right, top, u1, v0 });*/

		verticesList.push_back({ 0.0f, left,  top,    u0, v0 }); // 0: 좌상
		verticesList.push_back({ 0.0f, right, top,    u1, v0 }); // 1: 우상
		verticesList.push_back({ 0.0f, left,  bottom, u0, v1 }); // 2: 좌하
		verticesList.push_back({ 0.0f, right, bottom, u1, v1 }); // 3: 우하


	}

	// 빈 문자열이면 버퍼는 유지하고 그리기만 생략
	if (verticesList.empty())
	{
		mTextVertexCount = 0;
		return true;
	}

	const UINT requiredCount = static_cast<UINT>(verticesList.size());
	mTextVertexCount = static_cast<UINT>(verticesList.size());

	if (!FontTextureBuffer || requiredCount > mTextVertexCapacity)
	{
		const UINT newCapacity = (std::max)(requiredCount, mTextVertexCapacity * 2);

		D3D11_BUFFER_DESC bufferDesc = {};
		bufferDesc.ByteWidth = newCapacity * sizeof(FVertexTextured);
		bufferDesc.Usage = D3D11_USAGE_DYNAMIC; // 동적 설정
		bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE; // 동적으로 CPU가 쓰기 가능

		ID3D11Buffer* newBuffer = nullptr;
		Device->CreateBuffer(&bufferDesc, nullptr, &newBuffer);
		UE_LOG(Log, Render,	"Font buffer CREATE: capacity %u -> %u",
			mTextVertexCapacity, newCapacity);

		// 새 버퍼 생성에 성공한 뒤 기존 버퍼를 교체
		ReleaseFontAtlasQuad();
		FontTextureBuffer = newBuffer;
		mTextVertexCapacity = newCapacity;

	}

	D3D11_MAPPED_SUBRESOURCE mappedTextFontData = {};

	// 하위 리소스에 포함된 데이터에 대한 포인터를 가져오고 해당 하위 리소스에 대한 GPU 액세스를 거부합니다.
	HRESULT hr = DeviceContext->Map(FontTextureBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedTextFontData);

/*	HRESULT hr = Device->CreateBuffer(&bufferDesc, &textFontData, &FontTextureBuffer);*/



	if (FAILED(hr))
	{
		mTextVertexCount = 0;
		return false;
	}

	memcpy(mappedTextFontData.pData, verticesList.data(),	verticesList.size() * sizeof(FVertexTextured));

	DeviceContext->Unmap(FontTextureBuffer, 0);

	const UINT fontCount = static_cast<UINT>(Text->size());

	if (!ensureFontIndexBuffer(fontCount))
		return false;

	mTextIndexCount = fontCount * 6;
	mTextVertexCount = requiredCount;
	return true;

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

		indices.push_back(base + 3);
		indices.push_back(base + 2);
		indices.push_back(base + 1);
	}

	D3D11_BUFFER_DESC desc = {};
	desc.ByteWidth = static_cast<UINT>(indices.size() * sizeof(UINT));
	desc.Usage = D3D11_USAGE_IMMUTABLE; // 
	desc.BindFlags = D3D11_BIND_INDEX_BUFFER; // 인덱스 버퍼라고 명시

	D3D11_SUBRESOURCE_DATA data = {};
	data.pSysMem = indices.data();

	ID3D11Buffer* newBuffer = nullptr;
	HRESULT hr = Device->CreateBuffer(&desc, &data, &newBuffer);

	if (FAILED(hr))
		return false;

	if (FontIndexBuffer)
		FontIndexBuffer->Release();

	FontIndexBuffer = newBuffer;
	mTextIndexCapacity = newCapacity;
	return true;
}

void URenderer::RenderFontTexture(const FMatrix& world, const FMatrix& viewProjection)
{
	if(!DeviceContext ||
		!FontAtlasShaderResoruceView || // 폰트 아틀라스 텍스처를 셰이더에 연결할 뷰가 없음
		!FontTextureBuffer ||				// 문자열의 정점 데이터가 담긴 GPU 버퍼가 없음
		!TextureVertexShader ||			// 정점 위치와 UV를 처리할 버텍스 셰이더가 없음
		!TexturePixelShader ||			// 폰트 텍스처를 읽어 픽셀 색상을 출력할 픽셀 셰이더가 없음
		!TextureInputLayout ||			// 정점의 위치·UV 메모리 배치를 설명하는 입력 레이아웃이 없음
		!TextureSamplerState ||			// 텍스처 필터링과 주소 처리 방식을 지정하는 샘플러가 없음
		!FontBlendState ||				// 폰트의 알파값으로 배경과 합성할 블렌드 상태가 없음
		!ConstantBuffer ||				// 변환 행렬과 색상 등을 셰이더에 전달할 상수 버퍼가 없음
		mTextVertexCount == 0)			// 그릴 문자열 정점이 없음
	{
		return;
	}
	if (!FontIndexBuffer || mTextIndexCount == 0)
		return;

	// 이 테스트에서 바꿀 렌더 상태를 보관한다.
	ID3D11RasterizerState* previousRasterizer = nullptr;
	ID3D11DepthStencilState* previousDepth = nullptr;
	ID3D11BlendState* previousBlend = nullptr;

	UINT previousStencilRef = 0;
	FLOAT previousBlendFactor[4] = {};
	UINT previousSampleMask = 0;

	DeviceContext->RSGetState(&previousRasterizer);
	DeviceContext->OMGetDepthStencilState(&previousDepth, &previousStencilRef);
	DeviceContext->OMGetBlendState(	&previousBlend,	previousBlendFactor,&previousSampleMask);

	// 깊이 설정
	DeviceContext->OMSetRenderTargets(1, &FrameBufferRTV, DepthStencilView);
	DeviceContext->OMSetDepthStencilState(DepthStencilState, 0);

	DeviceContext->RSSetState(RasterizerState[0]);
	//DeviceContext->OMSetBlendState(nullptr, nullptr, 0xffffffff);
	DeviceContext->OMSetBlendState(FontBlendState, nullptr, 0xffffffff);

	const UINT stride = sizeof(FVertexTextured);
	const UINT offset = 0;

	DeviceContext->IASetVertexBuffers(0, 1, &FontTextureBuffer, &stride, &offset);
	DeviceContext->IASetIndexBuffer(FontIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	DeviceContext->IASetInputLayout(TextureInputLayout);
	DeviceContext->IASetPrimitiveTopology(
		D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	DeviceContext->VSSetShader(TextureVertexShader, nullptr, 0);
	DeviceContext->PSSetShader(TexturePixelShader, nullptr, 0);

	DeviceContext->VSSetConstantBuffers(0, 1, &ConstantBuffer);
	DeviceContext->PSSetConstantBuffers(0, 1, &ConstantBuffer);

	// HLSL의 t0, s0에 각각 연결한다.
	DeviceContext->PSSetShaderResources(0, 1, &FontAtlasShaderResoruceView);
	DeviceContext->PSSetSamplers(0, 1, &TextureSamplerState);

	UpdateConstant(world, viewProjection, FVector4(1, 1, 1, 1));

	// 기존 방식
	//DeviceContext->Draw(mTextVertexCount, 0);
	// 인덱스 버퍼 방식
	DeviceContext->DrawIndexed(mTextIndexCount, 0, 0);
	//DeviceContext->Draw(6, 0);

	// 테스트 바인딩 해제
	ID3D11ShaderResourceView* nullSRV = nullptr;
	ID3D11SamplerState* nullSampler = nullptr;

	DeviceContext->PSSetShaderResources(0, 1, &nullSRV);
	DeviceContext->PSSetSamplers(0, 1, &nullSampler);

	// 기존 엔진의 출력 대상과 상태 복원
	DeviceContext->OMSetRenderTargets(1, &FrameBufferRTV, DepthStencilView);

	DeviceContext->OMSetDepthStencilState(DepthStencilState, 0);

	DeviceContext->RSSetState(previousRasterizer);
	DeviceContext->OMSetDepthStencilState(previousDepth, previousStencilRef);
	DeviceContext->OMSetBlendState(
		previousBlend,
		previousBlendFactor,
		previousSampleMask);

	if (previousRasterizer) previousRasterizer->Release();
	if (previousDepth) previousDepth->Release();
	if (previousBlend) previousBlend->Release();

	PrepareSimpleShader();
}

// 
void URenderer::ReleaseFontTexture()
{
	// 매번 해제하면 버퍼를 재사용 불가능
	//ReleaseFontAtlasQuad();

	if (TextureSamplerState)
	{
		TextureSamplerState->Release();
		TextureSamplerState = nullptr;
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
