#pragma once

#include <d3d11.h>
#include <d3dcompiler.h>
#include "Matrix.h"
#include "Vector.h"

#pragma comment(lib, "user32")
#pragma comment(lib, "d3d11")
#pragma comment(lib, "d3dcompiler")

// 1. Define the triangle vertices
struct FVertexSimple
{
    float x, y, z;    // Position
    float r, g, b, a; // Color

	FVector GetPosition() const { return FVector(x, y, z); }
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
	ID3D11Texture2D* DepthStencilBuffer = nullptr;			// 실제 깊이값이 저장될 메모리
	ID3D11DepthStencilView* DepthStencilView = nullptr;		// 그 메모리를 "출력 대상"으로 보는 뷰
	ID3D11DepthStencilState* DepthStencilState = nullptr;	// 깊이 테스트용 상태
	ID3D11DepthStencilState* StencilMarkState = nullptr;	// 스텐실


    FLOAT ClearColor[4] = { 0.025f, 0.025f, 0.025f, 1.0f };
    D3D11_VIEWPORT ViewportInfo;
    ID3D11VertexShader* SimpleVertexShader;
    ID3D11PixelShader* SimplePixelShader;
    ID3D11InputLayout* SimpleInputLayout;

    unsigned int Stride;

public:

	//create
	void Create(HWND hWindow);
	void CreateDeviceAndSwapChain(HWND hWindow);
	void CreateShader();
	void CreateFrameBuffer();
	ID3D11Buffer* CreateVertexBuffer(FVertexSimple* vertices, UINT ByteWidth);
	void CreateRasterizerState();
	void CreateConstantBuffer();
	void CreateDepthStencilBuffer();
	void CreateDepthStencilState();
	void CreateStencilMarkState();

	//release
	void Release();
	void ReleaseDeviceAndSwapChain();
	void ReleaseShader();
	void ReleaseFrameBuffer();
	void ReleaseVertexBuffer(ID3D11Buffer* vertexBuffer);
	void ReleaseRasterizerState();
	void ReleaseConstantBuffer();
	void ReleaseDepthStencilBuffer();
	void ReleaseDepthStencilState();

	//Rendering
	void Prepare(bool bWireFrame);
	void PrepareShader();
	void UpdateConstant(FMatrix world, FMatrix viewProjection);
	void RenderPrimitive(ID3D11Buffer* pBuffer, UINT numVertices);
	void SwapBuffer();

    //=============================================
	//해상도 변경 시 호출
	void OnResize(UINT Width, UINT Height);
};
