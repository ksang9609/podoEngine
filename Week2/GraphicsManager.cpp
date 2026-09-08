#include "GraphicsManager.h"

#include "Renderer.h"
#include "Camera.h"
#include "Console.h"

GraphicsManager::GraphicsManager(HWND hWindow)
	: mbWireFrame(false)
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

void GraphicsManager::Prepare(const FCamera *mCamera)
{
	mRenderer->Prepare(mbWireFrame);
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
		mRenderer->UpdateConstant(renderInfo.WorldTransformMatrix, mViewProjectionMatrix, renderInfo.Color);

		FBuffer* vertexBuffer = mBufferMap.Find(renderInfo.ePrimitive);
		if (vertexBuffer == nullptr)
		{
			UE_LOG("Error: Vertex buffer not found for primitive type.");
			continue;
		}
		mRenderer->RenderPrimitive(vertexBuffer->Buffer, vertexBuffer->SourceNum);
	}
}
void GraphicsManager::RenderOverlay(const TArray<FRenderInfo> renderInfos) //깊이버퍼 초기화
{
	mRenderer->ClearDepth();
	Render(renderInfos);
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

FVector GraphicsManager::GetPrimitiveCenter(EPrimitive type)
{
	switch (type)
	{
		case EPrimitive::EP_Sphere:	return FVector(0, 0, 0);
		case EPrimitive::EP_Cube:	return FVector(0, 0, 0);
		default:					return FVector(0, 0, 0);
	}
}

// 테두리가 월드 공간에서 차지할 두께. 물체 크기와 무관하게 항상 이만큼만 자란다.
static constexpr float OUTLINE_THICKNESS = 0.02f;

// 월드 공간 반지름이 worldHalfExtent인 축을 OUTLINE_THICKNESS 만큼 키우는 배율
static float GetOutlineAxisScale(float worldHalfExtent)
{
	if (worldHalfExtent <= SMALL_NUMBER)
	{
		return 1.0f;   // 납작하게 눌린 축은 건드리지 않는다. 안 그러면 배율이 발산한다
	}

	return 1.0f + OUTLINE_THICKNESS / worldHalfExtent;
}

FVector GraphicsManager::GetPrimitiveHalfExtent(EPrimitive type)
{
	switch (type)
	{
		case EPrimitive::EP_Sphere:	return FVector(1.0f, 1.0f, 1.0f);
		case EPrimitive::EP_Cube:	return FVector(0.5f, 0.5f, 0.5f);
		default:					return FVector(0.5f, 0.5f, 0.5f);
	}
}

void GraphicsManager::RenderHighLight(const FRenderInfo& RI)
{
	const FVector Center = GetPrimitiveCenter(RI.ePrimitive);
	const FVector HalfExtent = GetPrimitiveHalfExtent(RI.ePrimitive);

	// 1.02배처럼 비율로 키우면 테두리 두께가 물체 크기에 그대로 비례한다.
	// 축마다 월드 공간에서 OUTLINE_THICKNESS 만큼만 자라도록 배율을 따로 구한다.
	const FVector WorldScale(
		RI.WorldTransformMatrix.GetUnitAxis(EAxis::X).Length(),
		RI.WorldTransformMatrix.GetUnitAxis(EAxis::Y).Length(),
		RI.WorldTransformMatrix.GetUnitAxis(EAxis::Z).Length());

	const FVector OutlineScale(
		GetOutlineAxisScale(HalfExtent.x * WorldScale.x),
		GetOutlineAxisScale(HalfExtent.y * WorldScale.y),
		GetOutlineAxisScale(HalfExtent.z * WorldScale.z));

	const FMatrix Outline = FMatrix::Translation(FVector(-Center.x, -Center.y, -Center.z))
		* FMatrix::Scale(OutlineScale)
		* FMatrix::Translation(Center)
		* RI.WorldTransformMatrix;

	FBuffer vertexBuffer = mBufferMap[RI.ePrimitive];
	mRenderer->RenderHighlight(vertexBuffer.Buffer, vertexBuffer.SourceNum, mViewProjectionMatrix, Outline, RI);
}
