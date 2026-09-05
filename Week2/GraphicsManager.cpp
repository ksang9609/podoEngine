#include "GraphicsManager.h"

#include "URenderer.h"

GraphicsManager::GraphicsManager(HWND hWindow)
{
	mRenderer = new URenderer;
	mRenderer->Create(hWindow);
	mRenderer->CreateShader();
	mRenderer->CreateConstantBuffer();

	// Todo: Fix
	mCamera = new FCamera(FTransform({ -2.0f, 1.0f, 1.0f }, { 0, 30, 0 }, { 1, 1, 1 }));
	mCamera->LookAt({ 0, 0, 0 });   // center of NearCube

	mAspect = mRenderer->ViewportInfo.Width / mRenderer->ViewportInfo.Height;
	mFovRad = 1.047f; // 60
}

GraphicsManager::~GraphicsManager()
{
	for (auto& buffer : mBufferMap)
	{
		buffer.second.Buffer->Release();
	}

	mRenderer->ReleaseConstantBuffer();
	mRenderer->ReleaseShader();
	mRenderer->Release();

	delete mRenderer;
	delete mCamera;
}

void GraphicsManager::Prepare()
{
	mRenderer->Prepare();
	mRenderer->PrepareShader();

	// Todo: 
	FMatrix view = mCamera->GetViewMatrix();
	FMatrix projection = mCamera->GetProjectionMatrix(mAspect, mFovRad, 0.1f, 100.0f);
	mViewProjectionMatrix = view * projection;

	// 그리는 순서가 중요하다: 가까운 것을 먼저, 먼 것을 나중에.
	// 깊이 테스트가 켜져 있으면 나중에 그린 FarCube 가 깊이 비교에서 탈락해
	// NearCube(주황)가 앞에 남고, 꺼져 있으면 FarCube(파랑)가 그 위를 덮어쓴다.
	//mRenderer->UpdateConstantViewProjection(viewProjection);
}

void GraphicsManager::Render(FTransform worldTransformMatrix, EPrimitive ePrimitive)
{
	mRenderer->UpdateConstant(worldTransformMatrix.MakeMatrix(), mViewProjectionMatrix);

	FBuffer vertexBuffer = mBufferMap[ePrimitive];
	mRenderer->RenderPrimitive(vertexBuffer.Buffer, vertexBuffer.SourceNum);
}

void GraphicsManager::Display()
{
	mRenderer->SwapBuffer();
}

void GraphicsManager::CreateBuffer(EPrimitive ePrimitive, FVertexSimple* vertices, uint32 verticesSize)
{
	assert(vertices != nullptr);

	UINT numVertices = static_cast<UINT>(verticesSize / sizeof(FVertexSimple));
	ID3D11Buffer* vertexBuffer = mRenderer->CreateVertexBuffer(vertices, verticesSize);

	FBuffer buffer = { vertexBuffer, numVertices };
	mBufferMap.Add(ePrimitive, buffer);
}

URenderer* GraphicsManager::GetRenderer() const
{
	assert(mRenderer != nullptr);

	return mRenderer;
}
