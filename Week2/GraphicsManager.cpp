#include "GraphicsManager.h"

#include "Renderer.h"
#include "Camera.h"
#include "Console.h"

GraphicsManager::GraphicsManager(HWND hWindow)
{
	mRenderer = new URenderer;
	mRenderer->Create(hWindow);
	mRenderer->CreateShader();
	mRenderer->CreateConstantBuffer();

	mAspect = mRenderer->ViewportInfo.Width / mRenderer->ViewportInfo.Height;
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
}

void GraphicsManager::Prepare(bool bWireFrame, const FCamera *mCamera)
{
	mRenderer->Prepare(bWireFrame);
	mRenderer->PrepareShader();

	// Todo: 
	FMatrix view = mCamera->GetViewMatrix();
	FMatrix projection = mCamera->GetProjectionMatrix(mAspect, mCamera->mFovDegree, 0.1f, 100.0f);
	mViewProjectionMatrix = view * projection;

	// 그리는 순서가 중요하다: 가까운 것을 먼저, 먼 것을 나중에.
	// 깊이 테스트가 켜져 있으면 나중에 그린 FarCube 가 깊이 비교에서 탈락해
	// NearCube(주황)가 앞에 남고, 꺼져 있으면 FarCube(파랑)가 그 위를 덮어쓴다.
	//mRenderer->UpdateConstantViewProjection(viewProjection);
}

void GraphicsManager::Render(const TArray<FRenderInfo> renderInfos)
{
	for (const FRenderInfo& renderInfo : renderInfos)
	{
		mRenderer->UpdateConstant(renderInfo.WorldTransformMatrix, mViewProjectionMatrix);

		FBuffer* vertexBuffer = mBufferMap.Find(renderInfo.ePrimitive);
		if (vertexBuffer == nullptr)
		{
			UE_LOG("Error: Vertex buffer not found for primitive type.");
			continue;
		}
		mRenderer->RenderPrimitive(vertexBuffer->Buffer, vertexBuffer->SourceNum);
	}
}

/*
void GraphicsManager::Render(FTransform worldTransformMatrix, EPrimitive ePrimitive)
{
	mRenderer->UpdateConstant(worldTransformMatrix.MakeMatrix(), mViewProjectionMatrix);

	FBuffer vertexBuffer = mBufferMap[ePrimitive];
	mRenderer->RenderPrimitive(vertexBuffer.Buffer, vertexBuffer.SourceNum);
}
*/

void GraphicsManager::Display()
{
	mRenderer->SwapBuffer();
}

void GraphicsManager::Update(float deltaTime)
{
	mAspect = mRenderer->ViewportInfo.Width / mRenderer->ViewportInfo.Height;
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
