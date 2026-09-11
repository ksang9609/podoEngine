#pragma once

#include <d3d11.h>
#include <d3dcompiler.h>
#include "Matrix.h"
#include "Vector.h"
#include "RenderInfo.h"

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
	FVector4 Tint;          // rgb = 색, a = 섞는 비율
};

struct FVertexTextured
{
	float x, y, z;
	float u, v;
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
	ID3D11DepthStencilState* StencilMarkState = nullptr;	// 스텐실에 1 마킹용 상태
	ID3D11DepthStencilState* StencilOutlineState = nullptr; // 아웃라인 그리기용
	ID3D11BlendState* NoColorWriteBlendState = nullptr;		// 스텐실만 찍고 색은 쓰지 않는 상태

	//ID3D11ShaderResourceView* TestTextureSRV = nullptr;
	ID3D11ShaderResourceView* FontAtlasSRV = nullptr;
	// 테스트용
	ID3D11Buffer* TestQuadBuffer = nullptr;
	ID3D11VertexShader* TextureVertexShader = nullptr;
	ID3D11PixelShader* TexturePixelShader = nullptr;
	ID3D11InputLayout* TextureInputLayout = nullptr;
	ID3D11SamplerState* TextureSamplerState = nullptr;
	


    FLOAT ClearColor[4] = { 0.025f, 0.025f, 0.025f, 1.0f };
    D3D11_VIEWPORT ViewportInfo;
    ID3D11VertexShader* SimpleVertexShader;
    ID3D11PixelShader* SimplePixelShader;
    ID3D11InputLayout* SimpleInputLayout;
	ID3D11VertexShader* LineSimpleVertexShader;
	ID3D11PixelShader* LineSimplePixelShader;
	ID3D11InputLayout* LineSimpleInputLayout;

	// 매 프레임 내용이 바뀌는 선분용. 메시 버퍼와 달리 IMMUTABLE이 아니라 DYNAMIC이다
	ID3D11Buffer* LineVertexBuffer = nullptr;
	uint32 LineVertexCapacity = 0;

	ID3D11Buffer* LineIndexBuffer = nullptr;
	uint32 LineIndexCapacity = 0;

    unsigned int Stride;

public:

	//create
	void Create(HWND hWindow);
	void CreateDeviceAndSwapChain(HWND hWindow);
	void CreateShader();
	void CreateFrameBuffer();
	ID3D11Buffer* CreateVertexBuffer(FVertexSimple* vertices, UINT ByteWidth);
	void CreateLineVertexBuffer(uint32 maxVertices);
	void CreateLineIndexBuffer(uint32 maxIndices);
	void CreateRasterizerState();
	void CreateConstantBuffer();
	void CreateDepthStencilBuffer(UINT width, UINT height);

	void CreateDepthStencilState();
	void CreateStencilMarkState();
	void CreateStencilOutlineState();
	void CreateNoColorWriteBlendState();
	bool CreateFontAtlasTexture();

	// test
	bool CreateTestQuad();
	void RenderTexture(const FMatrix& world, const FMatrix& viewProjection);

	//release
	void Release();
	void ReleaseDeviceAndSwapChain();
	void ReleaseShader();
	void ReleaseFrameBuffer();
	void ReleaseVertexBuffer(ID3D11Buffer* vertexBuffer);
	void ReleaseLineVertexBuffer();
	void ReleaseLineIndexBuffer();
	void ReleaseRasterizerState();
	void ReleaseConstantBuffer();
	void ReleaseDepthStencilBuffer();
	void ReleaseDepthStencilState();
	void ReleaseBlendState();
	void ReleaseTexture();
	void ReleaseFontAtlasTexture();

	//Update
	void RSUpdateState();

	//Rendering
	void Prepare(bool bWireFrame);
	void PrepareShader();
	void PrepareLineShader();
	void UpdateConstant(FMatrix world, FMatrix viewProjection, FVector4 tint = FVector4(0, 0, 0, 0));
	void RenderPrimitive(ID3D11Buffer* pBuffer, UINT numVertices);
	void RenderLines(const FVertexSimple* vertices, uint32 numVertices, const uint32* indices, uint32 numindices);
	void RenderHighlight(ID3D11Buffer* pBuffer, uint32 Num, FMatrix mViewProjectionMatrix, FMatrix OutlineMatrix, const FMatrix originalMatrix);
	void SwapBuffer();


	//Initialize
	void ClearDepth();
    //=============================================
	//해상도 변경 시 호출
	//void OnResize(UINT Width, UINT Height);
	void OnResize(UINT width, UINT height, float viewportWidth, float viewportHeight);
};
