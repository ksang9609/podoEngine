#include "Renderer.h"

#include <fstream>
#include <filesystem>
#include <vector>

void URenderer::Create(HWND hWindow)
{
	CreateDeviceAndSwapChain(hWindow);
	CreateFrameBuffer();
	//CreateDepthStencilBuffer();

	if (!CreateTestTexture())
	{
		OutputDebugStringA("CreateTestTexture failed.\n");
	}

	if (!CreateTestQuad())
    {
        OutputDebugStringA("CreateTestTexture failed.\n");
    }

	CreateDepthStencilState();
	CreateStencilMarkState();
	CreateStencilOutlineState();
	CreateNoColorWriteBlendState();
	CreateRasterizerState();
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

bool URenderer::CreateTestTexture()
{
	if (!Device)
	{
		return false;
	}

	// RGBA 순서. 위쪽 행부터 왼쪽 오른쪽으로 저장한다.
	const unsigned char pixels[] =
	{
		255,   0,   0, 255, // 좌상단: 빨강
		  0, 255,   0, 255, // 우상단: 초록

		  0,   0, 255, 255, // 좌하단: 파랑
		255, 255, 255, 255, // 우하단: 흰색
	};

	// GPU에 만들 이미지의 구조
	D3D11_TEXTURE2D_DESC desc = {};

	desc.Width = 2;
	desc.Height = 2;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.Usage = D3D11_USAGE_IMMUTABLE;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	// 생성할 때 전달할 픽셀 데이터
	D3D11_SUBRESOURCE_DATA initialData = {};

	initialData.pSysMem = pixels;
	initialData.SysMemPitch = 2 * 4; // 한 행: 2픽셀 × 4바이트

	ID3D11Texture2D* texture = nullptr;

	HRESULT hr = Device->CreateTexture2D(&desc,	&initialData, &texture);

	if (FAILED(hr))
	{
		return false;
	}

	// 텍스처를 셰이더에서 읽을 수 있는 뷰 생성
	ID3D11ShaderResourceView* srv = nullptr;

	hr = Device->CreateShaderResourceView(texture, nullptr,	&srv);

	// SRV가 텍스처 참조를 유지하므로 지역 참조는 해제
	texture->Release();

	if (FAILED(hr))
	{
		return false;
	}

	// 성공한 경우에만 기존 텍스처 교체
	ReleaseTestTexture();
	TestTextureSRV = srv;

	return true;
}

void URenderer::ReleaseTestTexture()
{
	if (TestTextureSRV)
	{
		TestTextureSRV->Release();
		TestTextureSRV = nullptr;
	}
}

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
	ReleaseTestQuad();
	ReleaseTestTexture();

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

	D3DCompileFromFile(L"ShaderW0.hlsl", nullptr, nullptr, "mainVS", "vs_5_0", 0, 0, &vertexshaderCSO, nullptr);

	Device->CreateVertexShader(vertexshaderCSO->GetBufferPointer(), vertexshaderCSO->GetBufferSize(), nullptr, &SimpleVertexShader);

	D3DCompileFromFile(L"ShaderW0.hlsl", nullptr, nullptr, "mainPS", "ps_5_0", 0, 0, &pixelshaderCSO, nullptr);

	Device->CreatePixelShader(pixelshaderCSO->GetBufferPointer(), pixelshaderCSO->GetBufferSize(), nullptr, &SimplePixelShader);

	D3DCompileFromFile(L"ShaderLine.hlsl", nullptr, nullptr, "mainVS", "vs_5_0", 0, 0, &LinevertexshaderCSO, nullptr);

	Device->CreateVertexShader(LinevertexshaderCSO->GetBufferPointer(), LinevertexshaderCSO->GetBufferSize(), nullptr, &LineSimpleVertexShader);

	D3DCompileFromFile(L"ShaderLine.hlsl", nullptr, nullptr, "mainPS", "ps_5_0", 0, 0, &LinepixelshaderCSO, nullptr);

	Device->CreatePixelShader(LinepixelshaderCSO->GetBufferPointer(), LinepixelshaderCSO->GetBufferSize(), nullptr, &LineSimplePixelShader);

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

	Device->CreateInputLayout(layout, ARRAYSIZE(layout), vertexshaderCSO->GetBufferPointer(), vertexshaderCSO->GetBufferSize(), &SimpleInputLayout);
	Device->CreateInputLayout(Linelayout, ARRAYSIZE(Linelayout), LinevertexshaderCSO->GetBufferPointer(), LinevertexshaderCSO->GetBufferSize(), &LineSimpleInputLayout);

	Stride = sizeof(FVertexSimple);

	vertexshaderCSO->Release();
	pixelshaderCSO->Release();
	LinevertexshaderCSO->Release();
	LinepixelshaderCSO->Release();
}

void URenderer::ReleaseShader()
{
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

void URenderer::PrepareShader()
{
	DeviceContext->VSSetShader(SimpleVertexShader, nullptr, 0);
	DeviceContext->PSSetShader(SimplePixelShader, nullptr, 0);
	DeviceContext->IASetInputLayout(SimpleInputLayout);

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

void URenderer::RenderPrimitive(ID3D11Buffer* pBuffer, UINT numVertices)
{
	UINT offset = 0;
	DeviceContext->IASetVertexBuffers(0, 1, &pBuffer, &Stride, &offset);
	DeviceContext->Draw(numVertices, 0);
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
	DeviceContext->IASetVertexBuffers(0, 1, &LineVertexBuffer, &Stride, &offset);
	DeviceContext->IASetIndexBuffer(LineIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	PrepareLineShader();
	DeviceContext->DrawIndexed(numindices, 0, 0);

	PrepareShader();
	DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void URenderer::RenderHighlight(ID3D11Buffer* pBuffer, uint32 Num, FMatrix mViewProjectionMatrix, FMatrix Outline, const FRenderInfo& RI)
{
	// (a) 스텐실에 1 마킹. 색은 쓰지 않으므로 화면 변화 없음.
	//     다른 오브젝트에 가려진 부분도 반드시 마킹해야 한다. 여기서 빠지면
	//     (b)의 != 1 조건을 통과해 버려서 겹친 영역 전체가 단색으로 칠해진다.
	DeviceContext->OMSetBlendState(NoColorWriteBlendState, nullptr, 0xffffffff);
	DeviceContext->OMSetDepthStencilState(StencilMarkState, 1);
	UpdateConstant(RI.WorldTransformMatrix, mViewProjectionMatrix);
	RenderPrimitive(pBuffer, Num);

	// (b) 확대판을 단색으로. 스텐실 != 1 인 곳만 통과 -> 테두리
	DeviceContext->OMSetBlendState(nullptr, nullptr, 0xffffffff);
	DeviceContext->OMSetDepthStencilState(StencilOutlineState, 1);
	UpdateConstant(Outline, mViewProjectionMatrix, FVector4(1.f, 0.6f, 0.f, 1.f));
	RenderPrimitive(pBuffer, Num);

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


// 테스트용 나중에 지울 예정

bool URenderer::CreateTestQuad()
{
	if (!Device)
		return false;

	ReleaseTestQuad();

	// 시계 방향 삼각형 두 개.
	const FVertexTextured vertices[] =
	{
		// 위치                 UV
		{ -0.5f,  0.5f, 0.5f,  0, 0 }, // 좌상
		{ 0.5f,  0.5f, 0.5f,  1, 0 }, // 우상
		{ -0.5f, -0.5f, 0.5f,  0, 1 }, // 좌하

		{ -0.5f, -0.5f, 0.5f,  0, 1 }, // 좌하
		{ 0.5f,  0.5f, 0.5f,  1, 0 }, // 우상
		{ 0.5f, -0.5f, 0.5f,  1, 1 }, // 우하
	};

	ID3DBlob* vsCode = nullptr;
	ID3DBlob* psCode = nullptr;

	bool success = false;

	do
	{
		// HLSL 컴파일
		HRESULT hr = D3DCompileFromFile(L"ShaderTexture.hlsl",	nullptr, nullptr, "mainVS", "vs_5_0", 0, 0, &vsCode, nullptr);

		if (FAILED(hr))
			break;

		hr = D3DCompileFromFile(L"ShaderTexture.hlsl",	nullptr, nullptr, "mainPS", "ps_5_0", 0, 0, &psCode, nullptr);

		if (FAILED(hr))
			break;

		// GPU 셰이더 생성
		hr = Device->CreateVertexShader(vsCode->GetBufferPointer(),	vsCode->GetBufferSize(), nullptr, &TestQuadVS);

		if (FAILED(hr))
			break;

		hr = Device->CreatePixelShader(psCode->GetBufferPointer(),	psCode->GetBufferSize(), nullptr, &TestQuadPS);

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
			&TestQuadLayout);

		if (FAILED(hr))
			break;

		// 고정된 정점 6개를 GPU에 저장
		D3D11_BUFFER_DESC bufferDesc = {};
		bufferDesc.ByteWidth = sizeof(vertices);
		bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

		D3D11_SUBRESOURCE_DATA initialData = {};
		initialData.pSysMem = vertices;

		hr = Device->CreateBuffer(&bufferDesc,	&initialData, &TestQuadBuffer);

		if (FAILED(hr))
			break;

		// 픽셀 경계가 선명하도록 POINT 샘플링
		D3D11_SAMPLER_DESC samplerDesc = {};
		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT; // 
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		samplerDesc.MaxAnisotropy = 1;
		samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

		hr = Device->CreateSamplerState(&samplerDesc,	&TestQuadSampler);

		if (FAILED(hr))
			break;

		success = true;
	} while (false);

	if (vsCode) vsCode->Release();
	if (psCode) psCode->Release();

	if (!success)
		ReleaseTestQuad();

	return success;
}

void URenderer::RenderTestQuad()
{
	if (!TestTextureSRV ||
		!TestQuadBuffer ||
		!TestQuadVS ||
		!TestQuadPS ||
		!TestQuadLayout ||
		!TestQuadSampler)
	{
		return;
	}

	// 이 테스트에서 바꿀 렌더 상태를 보관한다.
	ID3D11RasterizerState* previousRasterizer = nullptr;
	ID3D11DepthStencilState* previousDepth = nullptr;
	ID3D11BlendState* previousBlend = nullptr;

	UINT previousStencilRef = 0;
	FLOAT previousBlendFactor[4] = {};
	UINT previousSampleMask = 0;

	DeviceContext->RSGetState(&previousRasterizer);
	DeviceContext->OMGetDepthStencilState(
		&previousDepth, &previousStencilRef);
	DeviceContext->OMGetBlendState(
		&previousBlend,
		previousBlendFactor,
		&previousSampleMask);

	// 화면 테스트이므로 깊이 버퍼를 연결하지 않는다.
	DeviceContext->OMSetRenderTargets(
		1, &FrameBufferRTV, nullptr);

	DeviceContext->RSSetState(RasterizerState[0]);
	DeviceContext->OMSetBlendState(
		nullptr, nullptr, 0xffffffff);

	const UINT stride = sizeof(FVertexTextured);
	const UINT offset = 0;

	DeviceContext->IASetVertexBuffers(
		0, 1, &TestQuadBuffer, &stride, &offset);

	DeviceContext->IASetInputLayout(TestQuadLayout);
	DeviceContext->IASetPrimitiveTopology(
		D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	DeviceContext->VSSetShader(TestQuadVS, nullptr, 0);
	DeviceContext->PSSetShader(TestQuadPS, nullptr, 0);

	// HLSL의 t0, s0에 각각 연결한다.
	DeviceContext->PSSetShaderResources(0, 1, &TestTextureSRV);
	DeviceContext->PSSetSamplers(0, 1, &TestQuadSampler);

	DeviceContext->Draw(6, 0);

	// 테스트 바인딩 해제
	ID3D11ShaderResourceView* nullSRV = nullptr;
	ID3D11SamplerState* nullSampler = nullptr;

	DeviceContext->PSSetShaderResources(0, 1, &nullSRV);
	DeviceContext->PSSetSamplers(0, 1, &nullSampler);

	// 기존 엔진의 출력 대상과 상태 복원
	DeviceContext->OMSetRenderTargets(
		1, &FrameBufferRTV, DepthStencilView);

	DeviceContext->RSSetState(previousRasterizer);
	DeviceContext->OMSetDepthStencilState(
		previousDepth, previousStencilRef);
	DeviceContext->OMSetBlendState(
		previousBlend,
		previousBlendFactor,
		previousSampleMask);

	if (previousRasterizer) previousRasterizer->Release();
	if (previousDepth) previousDepth->Release();
	if (previousBlend) previousBlend->Release();

	PrepareShader();
}

void URenderer::ReleaseTestQuad()
{
	if (TestQuadBuffer)
	{
		TestQuadBuffer->Release();
		TestQuadBuffer = nullptr;
	}

	if (TestQuadVS)
	{
		TestQuadVS->Release();
		TestQuadVS = nullptr;
	}

	if (TestQuadPS)
	{
		TestQuadPS->Release();
		TestQuadPS = nullptr;
	}

	if (TestQuadLayout)
	{
		TestQuadLayout->Release();
		TestQuadLayout = nullptr;
	}

	if (TestQuadSampler)
	{
		TestQuadSampler->Release();
		TestQuadSampler = nullptr;
	}
}
