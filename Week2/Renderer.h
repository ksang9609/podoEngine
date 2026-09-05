#pragma once

#include <d3d11.h>
#include <d3dcompiler.h>
#include "Matrix.h"

#pragma comment(lib, "user32")
#pragma comment(lib, "d3d11")
#pragma comment(lib, "d3dcompiler")

// 1. Define the triangle vertices
struct FVertexSimple
{
    float x, y, z;    // Position
    float r, g, b, a; // Color
};

struct FConstants
{
	FMatrix World; //Model
	FMatrix ViewProjection;

};


class URenderer
{
public:
    ID3D11Device* Device = nullptr;
    ID3D11DeviceContext* DeviceContext = nullptr;
    IDXGISwapChain* SwapChain = nullptr;

    ID3D11Texture2D* FrameBuffer = nullptr;
    ID3D11RenderTargetView* FrameBufferRTV = nullptr;
	ID3D11RasterizerState* RasterizerState[2] = {};
    ID3D11Buffer* ConstantBuffer = nullptr;
	ID3D11Texture2D* DepthStencilBuffer = nullptr;		// 실제 깊이값이 저장될 메모리
	ID3D11DepthStencilView* DepthStencilView = nullptr;  // 그 메모리를 "출력 대상"으로 보는 뷰
	ID3D11DepthStencilState* DepthStencilState = nullptr;  // 깊이 테스트 규칙(켬/끔, 비교 함수)

    FLOAT ClearColor[4] = { 0.025f, 0.025f, 0.025f, 1.0f };
    D3D11_VIEWPORT ViewportInfo;
    ID3D11VertexShader* SimpleVertexShader;
    ID3D11PixelShader* SimplePixelShader;
    ID3D11InputLayout* SimpleInputLayout;

    unsigned int Stride;

public:
	void Create(HWND hWindow)
	{
		CreateDeviceAndSwapChain(hWindow);   // ViewportInfo가 여기서 채워짐
		CreateFrameBuffer();
		CreateDepthStencilBuffer();          // ← ViewportInfo를 쓰므로 반드시 위 두 개 뒤
		CreateDepthStencilState();
		CreateRasterizerState();
	}

    void CreateDeviceAndSwapChain(HWND hWindow)
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

    void ReleaseDeviceAndSwapChain()
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

    void CreateFrameBuffer()
    {
        SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&FrameBuffer);

        D3D11_RENDER_TARGET_VIEW_DESC framebufferRTVdesc = {};
        framebufferRTVdesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
        framebufferRTVdesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

        Device->CreateRenderTargetView(FrameBuffer, &framebufferRTVdesc, &FrameBufferRTV);
    }

    void ReleaseFrameBuffer()
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

    ID3D11Buffer* CreateVertexBuffer(FVertexSimple* vertices, UINT ByteWidth)
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

    void ReleaseVertexBuffer(ID3D11Buffer* vertexBuffer)
    {
        vertexBuffer->Release();
    }

    void CreateRasterizerState()
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

	void ReleaseRasterizerState()
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
    void Release()
    {
		ReleaseRasterizerState();

        DeviceContext->OMSetRenderTargets(0, nullptr, nullptr);

		ReleaseDepthStencilBuffer();
        ReleaseFrameBuffer();
        ReleaseDeviceAndSwapChain();
    }

    void SwapBuffer()
    {
        SwapChain->Present(1, 0);
    }

    void CreateShader()
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

    void ReleaseShader()
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

    void Prepare(bool bWireFrame)
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

    void PrepareShader()
    {
        DeviceContext->VSSetShader(SimpleVertexShader, nullptr, 0);
        DeviceContext->PSSetShader(SimplePixelShader, nullptr, 0);
        DeviceContext->IASetInputLayout(SimpleInputLayout);

        if (ConstantBuffer)
        {
            DeviceContext->VSSetConstantBuffers(0, 1, &ConstantBuffer);
        }
    }

    void RenderPrimitive(ID3D11Buffer* pBuffer, UINT numVertices)
    {
        UINT offset = 0;
        DeviceContext->IASetVertexBuffers(0, 1, &pBuffer, &Stride, &offset);
        DeviceContext->Draw(numVertices, 0);
    }


    //=============================================
  
    void CreateConstantBuffer()
    {
        D3D11_BUFFER_DESC constantbufferdesc = {};
        constantbufferdesc.ByteWidth = sizeof(FConstants) + 0xf & 0xfffffff0; // ensure constant buffer size is multiple of 16 bytes
        constantbufferdesc.Usage = D3D11_USAGE_DYNAMIC; // will be updated from CPU every frame
        constantbufferdesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        constantbufferdesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

        Device->CreateBuffer(&constantbufferdesc, nullptr, &ConstantBuffer);
    }

    void ReleaseConstantBuffer()
    {
        if (ConstantBuffer)
        {
            ConstantBuffer->Release();
            ConstantBuffer = nullptr;
        }
    }

	void CreateDepthStencilBuffer()
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

	void CreateDepthStencilState()
	{
		D3D11_DEPTH_STENCIL_DESC desc = {};
		desc.DepthEnable = TRUE;                          // 깊이 테스트 켜기
		desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;    // 통과한 픽셀의 z를 기록
		desc.DepthFunc = D3D11_COMPARISON_LESS;         // 더 가까우면(작으면) 통과
		desc.StencilEnable = FALSE;

		Device->CreateDepthStencilState(&desc, &DepthStencilState);
	}

	void ReleaseDepthStencilBuffer()
	{
		if (DepthStencilView) { DepthStencilView->Release();   DepthStencilView = nullptr; }
		if (DepthStencilBuffer) { DepthStencilBuffer->Release(); DepthStencilBuffer = nullptr; }
		if (DepthStencilState) { DepthStencilState->Release();  DepthStencilState = nullptr; }
	}

	void UpdateConstant(FMatrix world, FMatrix viewProjection)
	{
		if (ConstantBuffer)
		{
			D3D11_MAPPED_SUBRESOURCE constantbufferMSR;

			DeviceContext->Map(ConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &constantbufferMSR); // update constant buffer every frame
			FConstants* constants = (FConstants*)constantbufferMSR.pData;
			{
				constants->World = world;
				constants->ViewProjection = viewProjection;
			}
			DeviceContext->Unmap(ConstantBuffer, 0);
		}
	}

	/*
    void UpdateConstantWorld(FMatrix World)
    {
        if (ConstantBuffer)
        {
            D3D11_MAPPED_SUBRESOURCE constantbufferMSR;

            DeviceContext->Map(ConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &constantbufferMSR); // update constant buffer every frame
            FConstants* constants = (FConstants*)constantbufferMSR.pData;
            {
				constants->World = World;
            }
            DeviceContext->Unmap(ConstantBuffer, 0);
        }
    }

	void UpdateConstantViewProjection(FMatrix ViewProjection)
	{
		if (ConstantBuffer)
		{
			D3D11_MAPPED_SUBRESOURCE constantbufferMSR;

			DeviceContext->Map(ConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &constantbufferMSR); // update constant buffer every frame
			FConstants* constants = (FConstants*)constantbufferMSR.pData;
			{
				constants->ViewProjection = ViewProjection;
			}
			DeviceContext->Unmap(ConstantBuffer, 0);
		}
	}
	*/
};
