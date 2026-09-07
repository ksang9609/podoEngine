#include "Renderer.h"

void URenderer::Create(HWND hWindow)
{
	CreateDeviceAndSwapChain(hWindow);
	CreateFrameBuffer();
	CreateDepthStencilBuffer();
	CreateDepthStencilState();
	CreateStencilMarkState();
	CreateStencilOutlineState();
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

	D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
		D3D11_CREATE_DEVICE_BGRA_SUPPORT | D3D11_CREATE_DEVICE_DEBUG,
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
	ReleaseFrameBuffer();
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

	D3DCompileFromFile(L"ShaderW0.hlsl", nullptr, nullptr, "mainVS", "vs_5_0", 0, 0, &vertexshaderCSO, nullptr);

	Device->CreateVertexShader(vertexshaderCSO->GetBufferPointer(), vertexshaderCSO->GetBufferSize(), nullptr, &SimpleVertexShader);

	D3DCompileFromFile(L"ShaderW0.hlsl", nullptr, nullptr, "mainPS", "ps_5_0", 0, 0, &pixelshaderCSO, nullptr);

	Device->CreatePixelShader(pixelshaderCSO->GetBufferPointer(), pixelshaderCSO->GetBufferSize(), nullptr, &SimplePixelShader);

	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	Device->CreateInputLayout(layout, ARRAYSIZE(layout), vertexshaderCSO->GetBufferPointer(), vertexshaderCSO->GetBufferSize(), &SimpleInputLayout);

	Stride = sizeof(FVertexSimple);

	vertexshaderCSO->Release();
	pixelshaderCSO->Release();
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

void URenderer::RenderPrimitive(ID3D11Buffer* pBuffer, UINT numVertices)
{
	UINT offset = 0;
	DeviceContext->IASetVertexBuffers(0, 1, &pBuffer, &Stride, &offset);
	DeviceContext->Draw(numVertices, 0);
}

void URenderer::RenderHighlight(ID3D11Buffer* pBuffer, uint32 Num, FMatrix mViewProjectionMatrix, FMatrix Outline, const FRenderInfo& RI)
{
	// (a) 스텐실에 1 마킹. 색은 원본과 같으니 화면 변화 없음
	DeviceContext->OMSetDepthStencilState(StencilMarkState, 1);
	UpdateConstant(RI.WorldTransformMatrix, mViewProjectionMatrix);
	RenderPrimitive(pBuffer, Num);

	// (b) 확대판을 단색으로. 스텐실 != 1 인 곳만 통과 -> 테두리
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

void URenderer::CreateDepthStencilBuffer()
{
	D3D11_TEXTURE2D_DESC desc = {};
	desc.Width = (UINT)ViewportInfo.Width;   // 백버퍼와 크기가 정확히 같아야 함
	desc.Height = (UINT)ViewportInfo.Height;
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
	desc.DepthEnable = TRUE;							// 깊이 테스트 켜기
	desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;  // 깊이는 읽기만
	desc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;		// 더 가깝거나(작거나) 같으면 통과

	desc.StencilEnable = TRUE;							// 스텐실 사용
	desc.StencilReadMask = 0xFF;
	desc.StencilWriteMask = 0xFF;

	// 깊이 테스트를 통과한 픽셀에만 StencilRef를 기록
	desc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
	desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;   // 가려지면 안 씀
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


void URenderer::OnResize(UINT Width, UINT Height)
{
	if (!SwapChain || Width == 0 || Height == 0) return;
	if ((UINT)ViewportInfo.Width == Width && (UINT)ViewportInfo.Height == Height) return;

	//해상도에 의존하는 프레임 버퍼와 뎁스 스텐실 버퍼를 재생성한다.
	DeviceContext->OMSetRenderTargets(0, nullptr, nullptr);
	ReleaseFrameBuffer();
	ReleaseDepthStencilBuffer();

	HRESULT hr = SwapChain->ResizeBuffers(0, Width, Height, DXGI_FORMAT_UNKNOWN, 0);
	if (FAILED(hr)) return;

	DXGI_SWAP_CHAIN_DESC desc;
	SwapChain->GetDesc(&desc);
	ViewportInfo = { 0.0f, 0.0f, (float)desc.BufferDesc.Width, (float)desc.BufferDesc.Height, 0.0f, 1.0f };

	//상태는 이전에 생성한 걸 그대로 재사용
	CreateFrameBuffer();
	CreateDepthStencilBuffer();
}

void URenderer::ClearDepth()
{
	DeviceContext->ClearDepthStencilView(DepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
}
