#pragma once

#include <d3d11.h>
#include <d3dcompiler.h>

#include "Core/Math/Matrix.h"
#include "Core/Math/Vector.h"

#include "RenderInfo.h"
#include "VertexType.h"

#pragma comment(lib, "user32")
#pragma comment(lib, "d3d11")
#pragma comment(lib, "d3dcompiler")

struct FConstants
{
	FMatrix World; //Model
	FMatrix ViewProjection;
	FVector4 Tint;          // rgb = 색, a = 섞는 비율
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
	ID3D11ShaderResourceView* FontAtlasShaderResoruceView = nullptr;
	ID3D11Buffer* FontTextureBuffer = nullptr; // TODO: Rename to FontVertexBuffer
	ID3D11VertexShader* FontVertexShader = nullptr;
	ID3D11PixelShader* FontPixelShader = nullptr;
	ID3D11InputLayout* FontInputLayout = nullptr;
	ID3D11SamplerState* FontSamplerState = nullptr;
	ID3D11BlendState* FontBlendState = nullptr;
	ID3D11Buffer* FontIndexBuffer = nullptr;

	ID3D11VertexShader* PrimitiveTextureVertexShader = nullptr;
	ID3D11PixelShader* PrimitiveTexturePixelShader = nullptr;
	ID3D11InputLayout* PrimitiveTextureLayout = nullptr;
	ID3D11Buffer* CubeIndexBuffer = nullptr;     // 큐브 인덱스 저장


	//ID3D11ShaderResourceView* PrimitiveTextureSRV = nullptr;

	//ID3D11SamplerState* PrimitiveTextureSampler = nullptr;


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

    unsigned int StrideSimple;
    unsigned int StrideTextured;

public:

	//create
	void Create(HWND hWindow);
	void CreateDeviceAndSwapChain(HWND hWindow);
	void CreateShader();
	void CreateFrameBuffer();
	ID3D11Buffer* CreateVertexBuffer(FVertexSimple* vertices, UINT ByteWidth);
	ID3D11Buffer* CreateVertexBuffer(const FVertexTextured* vertices, UINT byteWidth);
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
	void CreateSamplerState(ID3D11SamplerState** outSamplerState);

	// font용
	bool CreateFontShader();
	bool CreateFontSamplerState();
	bool CreateFontBlendState();
	void RenderFontTexture(const FMatrix& world, const FMatrix& viewProjection);
	void ReleaseFontAtlasQuad();
	void ReleaseFontShader();
	bool CreateFontAtlasQuad(std::string* Text);

	bool CreateTestQuad(); // 기존의 쿼드를 그리는 함수(테스트 용)

	// texturedPrimitive용
	//void RenderTexture(const FMatrix& world, const FMatrix& viewProjection);

	
	//void RenderTexturedPrimitive(ID3D11Buffer* vertexBuffer, UINT numVertices, ID3D11ShaderResourceView* textureSRV);
	bool LoadTexture(const wchar_t* texturePath, ID3D11ShaderResourceView** outSRV);
	
	void ReleasePrimitiveTextureResources(
		ID3D11ShaderResourceView* textureSRV, ID3D11SamplerState* samplerState);
	//void RenderTexturedPrimitive(ID3D11Buffer* vertexBuffer, UINT numVertices);
	

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
	void ReleaseFontTexture();
	void ReleaseFontAtlasTexture();

	//Update
	void RSUpdateState();

	//Rendering
	void Prepare(bool bWireFrame);
	void PrepareSimpleShader();
	void PrepareTextureShader();
	void PrepareLineShader();
	void PrepareFontShader();

	/* Prepare methods for each rendering type */
	void PrepareSimplePrimitive();
	void PrepareTexturedPrimitive();
	void PrepareLine();
	void PrepareFont();
	void PrepareGizmo();
	void PrepareHighlight();

	void UpdateConstant(FMatrix world, FMatrix viewProjection, FVector4 tint = FVector4(0, 0, 0, 0));
	void UpdateFontBuffer(const TArray<FVertexTextured>& vertices, const TArray<uint32>& indices, uint32 numCharacter);

	void RenderSimplePrimitive(ID3D11Buffer* pBuffer, UINT numVertices);
	void RenderTexturePrimitive(ID3D11Buffer* pBuffer, UINT numVertices,
		ID3D11ShaderResourceView* texture, ID3D11SamplerState* samplerState, ID3D11Buffer* indexBuffer = nullptr);
	// Render Text in the FontTextureBuffer and FontIndexBuffer.
	// It doesn't recieve buffer parameters since it use the internal buffers.
	void RenderFontTexture(uint32 numCharacter);
	void RenderLines(const FVertexSimple* vertices, uint32 numVertices, const uint32* indices, uint32 numindices);
	void RenderHighlight(ID3D11Buffer* pBuffer, uint32 Num, FMatrix mViewProjectionMatrix, FMatrix OutlineMatrix, const FMatrix originalMatrix);

	void SwapBuffer();


	//Initialize
	void ClearDepth();
    //=============================================
	//해상도 변경 시 호출
	//void OnResize(UINT Width, UINT Height);
	void OnResize(UINT width, UINT height, float viewportWidth, float viewportHeight);

private:
	bool ensureFontIndexBuffer(UINT fontCount);
	UINT mTextVertexCount = 0;
	UINT mTextVertexCapacity = 0; // 저장할 수 있는 최대 정점 수
	UINT mTextIndexCount = 0;
	UINT mTextIndexCapacity = 0;

	/* Internal global rendering state */
	bool mbWireFrame = false;
};

