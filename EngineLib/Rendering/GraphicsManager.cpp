#include "GraphicsManager.h"
#include "Core/Math/Frustum.h" 

#include "Renderer.h"
#include "Camera.h"
#include "Editor/Console.h"
#include "Core/Container/TQueue.h"
#include "Engine/Components/PrimitiveComponent.h"
#include "Engine/Components/NameComponent.h"
#include "Engine/Actor.h"

FGraphicsManager::FGraphicsManager(HWND hWindow)
	: mbWireFrame(false)
	, mbPerspectiveProjection(true)
	, mProjectionRatio(1.0f)
{
	mRenderer = new URenderer;
	mRenderer->Create(hWindow);


	mAspect = mRenderer->ViewportInfo.Width / mRenderer->ViewportInfo.Height;
}

FGraphicsManager::~FGraphicsManager()
{
	for (auto& buffer : mBufferMap)
	{
		buffer.second.Buffer->Release();
	}

	for (auto& entry : mTexturedBufferMap)
	{
		if (entry.second.Buffer)
		{
			entry.second.Buffer->Release();
			entry.second.Buffer = nullptr;
		}
	}

	for (auto& [key, texture] : mPrimitiveTextureMap)
	{
		mRenderer->ReleasePrimitiveTextureResources(texture.SRV, texture.Sampler);
	}

	mTexturedBufferMap.Empty();
	mPrimitiveTextureMap.Empty();

	mRenderer->Release();

	delete mRenderer;
}

void FGraphicsManager::Prepare(const FCamera* mCamera)
{
	mRenderer->Prepare(mbWireFrame);

	// Cache view and projection matrices for rendering
	const float nearZ = 0.1f;
	const float farZ = 100.0f;

	float d = mCamera->mOrthoDistance;

	FMatrix view = mCamera->GetViewMatrix();
	FMatrix projection_u = mCamera->GetUnifiedProjectionMatrix(mAspect, mCamera->mFovDegree, d, nearZ, farZ, mProjectionRatio);

	mViewUnifiedProjectionMatrix = view * projection_u;

	// 하이라이트 두께를 화면 픽셀 기준으로 환산할 때 쓴다
	mCameraLocation = mCamera->Location;
	mCameraForward = mCamera->GetForwardVector();
	mCameraFovDegree = mCamera->mFovDegree;
	mCameraOrthoDistance = mCamera->mOrthoDistance;

	// 그리는 순서가 중요하다: 가까운 것을 먼저, 먼 것을 나중에.
	// 깊이 테스트가 켜져 있으면 나중에 그린 FarCube 가 깊이 비교에서 탈락해
	// NearCube(주황)가 앞에 남고, 꺼져 있으면 FarCube(파랑)가 그 위를 덮어쓴다.
	//mRenderer->UpdateConstantViewProjection(viewProjection);
}

//// TODO: remove outBillboardRenderQueue
//void QueueRenderQueue(
//	const TArray<FRenderInfo>& renderInfos,
//	TArray<const FRenderInfo*>& outSimpleRenderQueue,
//	TArray<const FRenderInfo*>& outTextureRenderQueue,
//	TArray<const FRenderInfo*>& outBillboardRenderQueue)
//{
//	for (const FRenderInfo& renderInfo : renderInfos)
//	{
//		if (renderInfo.ePrimitive == EPrimitive::EP_BillboardQuad)
//		{
//			outBillboardRenderQueue.Add(&renderInfo);
//		}
//		else if ((renderInfo.eRenderFlags & ERenderFlags::RF_TexturedPrimitive) == ERenderFlags::RF_None)
//		{
//			outTextureRenderQueue.Add(&renderInfo);
//		}
//		else
//		{
//			outSimpleRenderQueue.Add(&renderInfo);
//		}
//	}
//}
//
//bool RenderFlagMatch(ERenderFlags targetFlags, ERenderFlags renderFlags)
//{
//	return (static_cast<uint32>(targetFlags) & static_cast<uint32>(renderFlags)) != 0;
//}

// TODO: Combine worldaxis, bounding box into a single render queue type,
// since they are both line-based rendering and can be batched together.
// This will reduce the number of draw calls and improve performance.


void FGraphicsManager::updateRenderQueue(
	const TArray<FRenderInfo>& renderInfos,
	TMap<ERenderQueueType, TArray<const FRenderInfo*>>& outRenderQueueMap,
	const FFrustum* frustum)
{
	for (const FRenderInfo& renderInfo : renderInfos)
	{
		if (frustum != nullptr)
		{
			if (!frustum->Intersects(renderInfo.WorldBounds))
			{
				continue;
			}
		}

		ERenderFlags renderFlags = renderInfo.eRenderFlags;

		if (HasAllRenderFlags(renderFlags, ERenderFlags::RF_Primitive) &&
			HasShowFlag(EEngineShowFlags::SF_Primitives))
		{
			if (HasAllRenderFlags(renderFlags, ERenderFlags::RF_Texture))
			{
				outRenderQueueMap[RQT_TexturedPrimitive].Add(&renderInfo);
			}
			else
			{
				outRenderQueueMap[RQT_SimplePrimitive].Add(&renderInfo);
			}
		}
		if (HasAllRenderFlags(renderFlags, ERenderFlags::RF_BillboardText) &&
			HasShowFlag(EEngineShowFlags::SF_BillboardText))
		{
			outRenderQueueMap[RQT_BillboardText].Add(&renderInfo);
		}
		if (HasAllRenderFlags(renderFlags, ERenderFlags::RF_WorldAxis) &&
			HasShowFlag(EEngineShowFlags::SF_WorldAxis))
		{
			outRenderQueueMap[RQT_WorldAxis].Add(&renderInfo);
		}
		if (HasAllRenderFlags(renderFlags, ERenderFlags::RF_Gizmo))
		{
			outRenderQueueMap[RQT_Gizmo].Add(&renderInfo);
		}
		if (HasAllRenderFlags(renderFlags, ERenderFlags::RF_BoundingBox))
		{
			outRenderQueueMap[RQT_BoundingBox].Add(&renderInfo);
		}
	}
}

void FGraphicsManager::Render(
	const TArray<FRenderInfo>& scenerRenderInfos,
	const TArray<FRenderInfo>& gizmoRenderInfos,
	const TArray<FRenderInfo>& axisRenderInfos,
	const FCamera& camera,
	const AActor* selectedActor)
{

	Prepare(&camera);

	const FFrustum frustum = FFrustum::FrustumFromViewProjection(mViewUnifiedProjectionMatrix);

	// Prepare Render queue
	// renderInfos includes primtives, textured primitives, billboard, and gizmo render infos
	// Each render info is splitted into different render queues
	TMap<ERenderQueueType, TArray<const FRenderInfo*>> renderQueueMap;
	updateRenderQueue(scenerRenderInfos, renderQueueMap, &frustum);
	updateRenderQueue(gizmoRenderInfos, renderQueueMap, nullptr);
	updateRenderQueue(axisRenderInfos, renderQueueMap, nullptr);

	renderSimplePrimitive(renderQueueMap[RQT_SimplePrimitive], camera);
	renderTexturedPrimitive(renderQueueMap[RQT_TexturedPrimitive], camera);
	renderBillboardText(renderQueueMap[RQT_BillboardText], camera);

	// Line Buffer에 넣기전에 Buffer의 용량을 미리 지정하여 동적할당 방지
	CalculateLineBuffer(renderQueueMap[RQT_BoundingBox]);

	//월드 축. 액터 뒤에 그려서 같은 깊이 버퍼로 가려지게 한다 (기즈모와 달리 깊이를 지우지 않는다)
	renderWorldAxis(renderQueueMap[RQT_WorldAxis]);
	renderGrid();
	renderBoundingBox(renderQueueMap[RQT_BoundingBox], camera.GetRotation());
	FlushLines();

	//강조
	if (selectedActor)
	{
		FRenderInfo clickedRenderInfo;
		selectedActor->GetFirstRenderInfo(clickedRenderInfo);
		renderHighLight(clickedRenderInfo, camera);
	}

	/* Clear Depth */
	mRenderer->ClearDepth();

	// Gizmo
	renderGizmo(renderQueueMap[RQT_Gizmo], camera);
}

void FGraphicsManager::renderSimplePrimitive(const TArray<const FRenderInfo*>& renderInfos, const FCamera& camera)
{
	mRenderer->PrepareSimplePrimitive();
	for (const FRenderInfo* renderInfo : renderInfos)
	{
		FMatrix worldTransform = renderInfo->WorldTransformMatrix;
		mRenderer->UpdateConstant(worldTransform, mViewUnifiedProjectionMatrix, renderInfo->Color);
		FBuffer* vertexBuffer = mBufferMap.Find(renderInfo->ePrimitive);
		if (vertexBuffer == nullptr)
		{
			UE_LOG(Error, Render, "Vertex buffer not found for primitive type.");
			continue;
		}
		mRenderer->RenderSimplePrimitive(vertexBuffer->Buffer, vertexBuffer->SourceNum);
	}
}

void FGraphicsManager::renderGizmo(const TArray<const FRenderInfo*>& renderInfos, const FCamera& camera)
{
	mRenderer->PrepareGizmo();
	for (const FRenderInfo* renderInfo : renderInfos)
	{
		FMatrix worldTransform = renderInfo->WorldTransformMatrix;
		mRenderer->UpdateConstant(worldTransform, mViewUnifiedProjectionMatrix, renderInfo->Color);
		FBuffer* vertexBuffer = mBufferMap.Find(renderInfo->ePrimitive);
		if (vertexBuffer == nullptr)
		{
			UE_LOG(Error, Render, "Vertex buffer not found for primitive type.");
			continue;
		}
		mRenderer->RenderSimplePrimitive(vertexBuffer->Buffer, vertexBuffer->SourceNum);
	}
}

void FGraphicsManager::renderTexturedPrimitive(const TArray<const FRenderInfo*>& renderInfos, const FCamera& camera)
{
	mRenderer->PrepareTexturedPrimitive();
	for (const FRenderInfo* renderInfo : renderInfos)
	{
		FMatrix worldTransform = renderInfo->WorldTransformMatrix;
		mRenderer->UpdateConstant(worldTransform, mViewUnifiedProjectionMatrix, renderInfo->Color);
		FBuffer* vertexBuffer = mTexturedBufferMap.Find(renderInfo->ePrimitive);
		if (vertexBuffer == nullptr)
		{
			UE_LOG(Error, Render, "Error: Textured vertex buffer not found for primitive type.");
			continue;
		}
		FTexture* texture = mPrimitiveTextureMap.Find(renderInfo->ePrimitive);
		if (texture == nullptr)
		{
			UE_LOG(Error, Render, "Error: Primitive texture not found for primitive type.");
			continue;
		}
		/*mRenderer->RenderTexturePrimitive(vertexBuffer->Buffer, vertexBuffer->SourceNum,
			texture->SRV, texture->Sampler);*/

		ID3D11Buffer* indexBuffer = nullptr;
		UINT indexCount = 0;

		if (renderInfo->ePrimitive == EPrimitive::EP_Cube)
		{
			indexBuffer = mRenderer->CubeIndexBuffer;
			indexCount = 36;

			// indexBuffer = mRenderer->CubeIndexBuffer;
			if (!indexBuffer)
				continue;
		}
		else if (renderInfo->ePrimitive == EPrimitive::EP_Sphere)
		{
			indexBuffer = mRenderer->SphereIndexBuffer;
			indexCount = mRenderer->SphereIndexCount;

			if (!indexBuffer || indexCount == 0)
				continue;
		}

		mRenderer->RenderTexturePrimitive(
			vertexBuffer->Buffer,
			vertexBuffer->SourceNum,
			texture->SRV,
			texture->Sampler,
			indexBuffer, indexCount); // 마지막 인수에 전달
	}
}

void FGraphicsManager::renderBillboardText(const TArray<const FRenderInfo*>& renderInfos, const FCamera& camera)
{
	// Render Billboard Quads
	// TODO: Remove dedicated render path for billboard quads if possible
	mRenderer->PrepareFont();
	for (const FRenderInfo* renderInfo : renderInfos)
	{
		FMatrix worldTransform = renderInfo->GetTransformMatrix(camera.Rotation);
		mRenderer->UpdateConstant(worldTransform, mViewUnifiedProjectionMatrix, renderInfo->Color);
		mRenderer->UpdateFontBuffer(
			renderInfo->Textmesh->Vertices, renderInfo->Textmesh->Indices,
			renderInfo->Textmesh->TextNum);
		mRenderer->RenderFontTexture(renderInfo->Textmesh->TextNum);
	}
}

void FGraphicsManager::DrawLine(const FVector& start, const FVector& end, const FVector4& color)
{
	// 월드 좌표 그대로 넣는다. 그래서 그릴 때 World 행렬이 단위행렬이다
	uint32 mStartOffset = mLineVertices.Num();

	mLineVertices.Add({ start.x, start.y, start.z, color.x, color.y, color.z, color.w });
	mLineVertices.Add({ end.x,   end.y,   end.z,   color.x, color.y, color.z, color.w });

	// Index Buffer 업데이트
	mLineIndices.Add(mStartOffset);
	mLineIndices.Add(mStartOffset + 1);
}

void FGraphicsManager::DrawAABBLine(const FBoundingBox& bounds, const FVector4& color)
{
	const FVector3& boundsMin = bounds.min;
	const FVector3& boundsMax = bounds.max;

	const FVector3 corners[8] =
	{
		{ boundsMin.x, boundsMin.y, boundsMin.z },
		{ boundsMax.x, boundsMin.y, boundsMin.z },
		{ boundsMin.x, boundsMax.y, boundsMin.z },
		{ boundsMax.x, boundsMax.y, boundsMin.z },

		{ boundsMin.x, boundsMin.y, boundsMax.z },
		{ boundsMax.x, boundsMin.y, boundsMax.z },
		{ boundsMin.x, boundsMax.y, boundsMax.z },
		{ boundsMax.x, boundsMax.y, boundsMax.z }
	};

	const uint32 baseVertex =
		static_cast<uint32>(mLineVertices.Num());

	for (const FVector3& corner : corners)
	{
		mLineVertices.Add({
			corner.x, corner.y, corner.z,
			color.x, color.y, color.z, color.w
			});
	}

	static constexpr uint32 indices[] =
	{
		0, 1, 1, 3, 3, 2, 2, 0,
		4, 5, 5, 7, 7, 6, 6, 4,
		0, 4, 1, 5, 2, 6, 3, 7
	};

	for (uint32 index : indices)
	{
		mLineIndices.Add(baseVertex + index);
	}
}

void FGraphicsManager::renderWorldAxis(const TArray<const FRenderInfo*>& renderInfos)
{
	// far plane이 100이라 그 안쪽으로 잡아야 잘리지 않는다
	constexpr float AXIS_LENGTH = 50.0f;
	// 세 축이 원점에서 정확히 겹치면 깊이 다툼이 생긴다. 눈에 안 띌 만큼만 띄운다
	constexpr float AXIS_ORIGIN_GAP = 0.01f;
	// 음의 방향은 어둡게 깔아 +쪽과 구분한다 (언리얼 에디터와 같은 처리)
	constexpr float NEGATIVE_DIM = 0.25f;

	const FVector axisDirections[3] =
	{
		FVector(1.0f, 0.0f, 0.0f),
		FVector(0.0f, 1.0f, 0.0f),
		FVector(0.0f, 0.0f, 1.0f),
	};
	const FVector4 axisColors[3] =
	{
		FVector4(1.0f, 0.0f, 0.0f, 1.0f),   // X = 빨강
		FVector4(0.0f, 1.0f, 0.0f, 1.0f),   // Y = 초록
		FVector4(0.0f, 0.4f, 1.0f, 1.0f),   // Z = 파랑
	};

	for (const FRenderInfo* renderInfo : renderInfos)
	{
		for (int32 i = 0; i < 3; ++i)
		{
			const FVector& direction = axisDirections[i];
			const FVector4& color = axisColors[i];
			const FVector4 dimColor(
				color.x * NEGATIVE_DIM,
				color.y * NEGATIVE_DIM,
				color.z * NEGATIVE_DIM,
				color.w);

			DrawLine(direction * AXIS_ORIGIN_GAP, direction * AXIS_LENGTH, color);
			DrawLine(direction * -AXIS_ORIGIN_GAP, direction * -AXIS_LENGTH, dimColor);
		}
	}
}

void FGraphicsManager::renderGrid()
{
	int LineCount = (mgridExtent / 2) / mgridSpacing;
	for (int32 i = -LineCount; i <= LineCount;i++)
	{
		float Spaceline = i * mgridSpacing;
		if (HasShowFlag(EEngineShowFlags::SF_WorldAxis)) {
			if (Spaceline == 0) continue;
		}
		DrawLine(FVector3(Spaceline, -mgridExtent / 2.0f, 0), FVector3(Spaceline, mgridExtent / 2.0f, 0), FVector4(0.3f, 0.3f, 0.3f, 1.0f));  // Y축 기준 Grid
		DrawLine(FVector3(-mgridExtent / 2.0f, Spaceline, 0), FVector3(mgridExtent / 2.0f, Spaceline, 0), FVector4(0.3f, 0.3f, 0.3f, 1.0f)); // X축 기준 Grid
	}
}

void FGraphicsManager::renderBoundingBox(const TArray<const FRenderInfo*>& renderInfos, const FRotator& cameraRotation)
{
	for (const FRenderInfo* renderInfo : renderInfos)
	{
		if (renderInfo->ePrimitive == EPrimitive::EP_BillboardQuad)
		{
			if (!HasShowFlag(EEngineShowFlags::SF_BillboardText))
			{
				continue;
			}
		}
		else
		{
			if (!HasShowFlag(EEngineShowFlags::SF_Primitives))
			{
				continue;
			}
		}

		// Billboard는 카메라 회전이 실제 렌더 행렬에 포함되므로(카메라 방향에 따라 월드 변환이 바뀜)
		// 현재 카메라 기준으로 WorldBounds를 갱신
		const FBoundingBox bounds = renderInfo->ePrimitive == EPrimitive::EP_BillboardQuad
			? TransformBoundingBox(
				renderInfo->LocalBounds,
				renderInfo->GetTransformMatrix(cameraRotation))
			: renderInfo->WorldBounds;

		DrawAABBLine(
			bounds,
			FVector4(1.0f, 1.0f, 1.0f, 1.0f));
	}
}

void FGraphicsManager::FlushLines()
{
	if (mLineVertices.Num() == 0) return;

	mRenderer->PrepareLine();

	// 선분 좌표가 이미 월드 공간이라 World는 단위행렬.
	// Tint.a = 0 이면 셰이더의 lerp가 정점 색을 그대로 통과시킨다
	mRenderer->UpdateConstant(FMatrix::Identity, mViewUnifiedProjectionMatrix, FVector4(0, 0, 0, 0));
	mRenderer->RenderLines(&mLineVertices[0], mLineVertices.Num(), &mLineIndices[0], mLineIndices.Num());

	// 안 비우면 매 프레임 누적돼 버퍼가 넘친다. 용량은 유지한 채 개수만 0으로
	mLineVertices.Reset(LINE_VERTEX_CAPACITY);
	mLineIndices.Reset(LINE_INDEX_CAPACITY);
}

void FGraphicsManager::Display()
{
	//mRenderer->RenderFontTexture(
	//	FMatrix::Identity,
	//	FMatrix::Identity
	//);
	mRenderer->SwapBuffer();
}

void FGraphicsManager::Update(float deltaTime)
{
	mAspect = mRenderer->ViewportInfo.Width / mRenderer->ViewportInfo.Height;

	// 테스트용: deltaTime이 초 단위라는 전제
	//static float elapsed = 0.0f;
	//static size_t index = 0;

	//static std::string testTexts[] = {
	//	"ABC",
	//	"XYZ",
	//	"ABCDEFGHIJKLMNOPQRSTUVWXYZ",
	//	"Hi",
	//	"",
	//	"Back!",
	//	"sdffffffffffffffffffffffffffffffffffffffffffffff"
	//};

	//elapsed += deltaTime;

	//if (elapsed >= 1.0f)
	//{
	//	elapsed = 0.0f;

	//	if (!mRenderer->CreateFontAtlasQuad(&testTexts[index]))
	//	{
	//		UE_LOG(Error, Render, "Failed to update font text.");
	//	}

	//	index = (index + 1)
	//		% (sizeof(testTexts) / sizeof(testTexts[0]));
	//}
}

bool FGraphicsManager::IsPerspectiveProjection() const
{
	return mbPerspectiveProjection;
}

void FGraphicsManager::SetPerspectiveProjection(bool bPerspectiveProjection)
{
	mbPerspectiveProjection = bPerspectiveProjection;
}

void FGraphicsManager::CreateBuffer(EPrimitive ePrimitive, FVertexSimple* vertices, uint32 verticesSize)
{
	assert(vertices != nullptr);

	UINT numVertices = static_cast<UINT>(verticesSize / sizeof(FVertexSimple));
	ID3D11Buffer* vertexBuffer = mRenderer->CreateVertexBuffer(vertices, verticesSize);
	FVector3 LocalMin = FVector3(vertices[0].x, vertices[0].y, vertices[0].z);
	FVector3 LocalMax = FVector3(vertices[0].x, vertices[0].y, vertices[0].z);
	for (int i = 0;i < numVertices;i++)
	{
		LocalMin.x = min(LocalMin.x, vertices[i].x);
		LocalMin.y = min(LocalMin.y, vertices[i].y);
		LocalMin.z = min(LocalMin.z, vertices[i].z);
		LocalMax.x = max(LocalMax.x, vertices[i].x);
		LocalMax.y = max(LocalMax.y, vertices[i].y);
		LocalMax.z = max(LocalMax.z, vertices[i].z);
	} // AABB 렌더링에 필요한 LocalMin,Max 저장
	FBoundingBox LocalBound;
	LocalBound.min = LocalMin;
	LocalBound.max = LocalMax;
	FBuffer buffer = { vertexBuffer, numVertices, LocalBound }; // 버퍼에 저장하여 도형 하나당 한번씩만 캐싱 진행하도록 함
	mBufferMap.Add(ePrimitive, buffer);
}

void FGraphicsManager::CreateTexturedBuffer(EPrimitive ePrimitive, const FVertexTextured* vertices, uint32 verticesSize)
{
	if (!vertices || verticesSize == 0 ||
		verticesSize % sizeof(FVertexTextured) != 0)
	{
		UE_LOG(Log, Core, "Invalid textured vertex data.");
		return;
	}

	ID3D11Buffer* vertexBuffer = mRenderer->CreateVertexBuffer(vertices, verticesSize);

	if (!vertexBuffer)
	{
		UE_LOG(Log, Core, "Failed to create textured vertex buffer.");
		return;
	}

	FBuffer buffer = {};
	buffer.Buffer = vertexBuffer;
	buffer.SourceNum = static_cast<uint32>(verticesSize / sizeof(FVertexTextured));

	// 기존 색상용 버퍼처럼 로컬 AABB 계산
	buffer.LocalBounds.min = FVector3(vertices[0].x, vertices[0].y, vertices[0].z);
	buffer.LocalBounds.max = buffer.LocalBounds.min;

	/*for (uint32 i = 1; i < buffer.SourceNum; ++i)
	{
		const auto& v = vertices[i];

		buffer.LocalBounds.min.x = min(buffer.LocalBounds.min.x, v.x);
		buffer.LocalBounds.min.y = min(buffer.LocalBounds.min.y, v.y);
		buffer.LocalBounds.min.z = min(buffer.LocalBounds.min.z, v.z);

		buffer.LocalBounds.max.x = max(buffer.LocalBounds.max.x, v.x);
		buffer.LocalBounds.max.y = max(buffer.LocalBounds.max.y, v.y);
		buffer.LocalBounds.max.z = max(buffer.LocalBounds.max.z, v.z);
	}*/

	// 동일한 종류를 다시 등록한다면 이전 버퍼 해제
	if (FBuffer* previous = mTexturedBufferMap.Find(ePrimitive))
	{
		if (previous->Buffer)
			previous->Buffer->Release();
	}

	mTexturedBufferMap.Add(ePrimitive, buffer);
}

void FGraphicsManager::CreatePrimitiveTexture(EPrimitive ePrimitive, const wchar_t* texturePath)
{
	// TODO: Set string path to the texture later

	FTexture texture{};

	if (!mRenderer->LoadTexture(texturePath, &texture.SRV))
	{
		UE_LOG(Log, Core, "Failed to create primitive texture resources.");
	}

	mRenderer->CreateSamplerState(&texture.Sampler);

	mPrimitiveTextureMap.Add(ePrimitive, texture);
}

URenderer* FGraphicsManager::GetRenderer() const
{
	assert(mRenderer != nullptr);

	return mRenderer;
}

FVector FGraphicsManager::GetPrimitiveCenter(EPrimitive type)
{
	switch (type)
	{
	case EPrimitive::EP_Sphere:	return FVector(0, 0, 0);
	case EPrimitive::EP_Cube:	return FVector(0, 0, 0);
	default:					return FVector(0, 0, 0);
	}
}

// 테두리가 화면에서 차지할 두께(픽셀). 물체 크기와 카메라 거리 어느 쪽에도 영향받지 않는다.
static constexpr float OUTLINE_PIXELS = 3.0f;

// 월드 공간 반지름이 worldHalfExtent인 축을 worldThickness 만큼 키우는 배율
static float GetOutlineAxisScale(float worldHalfExtent, float worldThickness)
{
	if (worldHalfExtent <= SMALL_NUMBER)
	{
		return 1.0f;   // 납작하게 눌린 축은 건드리지 않는다. 안 그러면 배율이 발산한다
	}

	return 1.0f + worldThickness / worldHalfExtent;
}

FVector FGraphicsManager::GetPrimitiveHalfExtent(EPrimitive type)
{
	switch (type)
	{
	case EPrimitive::EP_Sphere:	return FVector(1.0f, 1.0f, 1.0f);
	case EPrimitive::EP_Cube:	return FVector(0.5f, 0.5f, 0.5f);
	default:					return FVector(0.5f, 0.5f, 0.5f);
	}
}

float FGraphicsManager::GetGridWidth() const
{
	return mgridSpacing;
}

void  FGraphicsManager::SetGridWidth(float width)
{
	mgridSpacing = width;
}


void FGraphicsManager::renderHighLight(const FRenderInfo& RI, const FCamera& camera)
{
	mRenderer->PrepareHighlight();

	const FVector Center = GetPrimitiveCenter(RI.ePrimitive);
	const FVector HalfExtent = GetPrimitiveHalfExtent(RI.ePrimitive);
	FMatrix worldTransformMatrix = RI.GetTransformMatrix(camera.Rotation);

	// 화면에서 OUTLINE_PIXELS 만큼 보이려면 이 깊이에서 월드로 얼마여야 하는지 환산한다.
	// 깊이 d에서 뷰포트가 담는 월드 높이가 2*d*tan(fov/2) 이므로, 그걸 픽셀 수로 나누면 픽셀당 월드 크기다.
	const FVector ObjectLocation = worldTransformMatrix.TransformPosition(Center);
	const float Depth = FVector::dot(ObjectLocation - mCameraLocation, mCameraForward);
	const float TanHalfFov = tanf(FMath::DegreesToRadians(mCameraFovDegree * 0.5f));
	const float effectiveDepth = FMath::Max(
		(1.0f - mProjectionRatio) * mCameraOrthoDistance + mProjectionRatio * Depth
		, 0.01f);
	//const float H = mbPerspectiveProjection ? 2.0f * Depth * TanHalfFov : 5.774f;
	const float H = 2.0f * effectiveDepth * TanHalfFov;
	const float WorldThickness = OUTLINE_PIXELS * H / mRenderer->ViewportInfo.Height;


	// 축마다 월드 공간에서 WorldThickness 만큼만 자라도록 배율을 따로 구한다.
	const FVector WorldScale(
		worldTransformMatrix.GetUnitAxis(EAxis::X).Length(),
		worldTransformMatrix.GetUnitAxis(EAxis::Y).Length(),
		worldTransformMatrix.GetUnitAxis(EAxis::Z).Length());

	FVector OutlineScale = {
		GetOutlineAxisScale(HalfExtent.x * WorldScale.x, WorldThickness),
		GetOutlineAxisScale(HalfExtent.y * WorldScale.y, WorldThickness),
		GetOutlineAxisScale(HalfExtent.z * WorldScale.z, WorldThickness) };


	const FMatrix Outline = FMatrix::Translation(FVector(-Center.x, -Center.y, -Center.z))
		* FMatrix::Scale(OutlineScale)
		* FMatrix::Translation(Center)
		* worldTransformMatrix;

	FBuffer vertexBuffer = mBufferMap[RI.ePrimitive];
	//if (mbPerspectiveProjection)
	//{
	//	mRenderer->RenderHighlight(vertexBuffer.Buffer, vertexBuffer.SourceNum, mViewProjectionMatrix, Outline, RI);
	//}
	//else
	//{
	//	mRenderer->RenderHighlight(vertexBuffer.Buffer, vertexBuffer.SourceNum, mViewOrthogonalProjectionMatrix, Outline, RI);
	//}
	mRenderer->RenderHighlight(vertexBuffer.Buffer, vertexBuffer.SourceNum, mViewUnifiedProjectionMatrix, Outline, worldTransformMatrix);
}


void FGraphicsManager::StartProjectionTransition(bool orthographic)
{
	mProjectionStartRatio = mProjectionRatio;
	mProjectionTargetRatio = orthographic ? 0.0f : 1.0f;
	mProjectionElapsed = 0.0f;

	mbProjectionTransitioning =
		mProjectionStartRatio != mProjectionTargetRatio;
}

bool FGraphicsManager::IsOrthographicTarget() const
{
	return mProjectionTargetRatio == 0.0f;
}

void FGraphicsManager::UpdateProjectionTransition(float deltaTime)
{
	if (!mbProjectionTransitioning)
	{
		return;
	}

	mProjectionElapsed += deltaTime;

	const float u = FMath::Clamp(
		mProjectionElapsed / mProjectionDuration, 0.0f, 1.0f);

	// Smoothstep interpolation for a smoother transition
	const float blend = u * u * (3.0f - 2.0f * u);

	mProjectionRatio = mProjectionStartRatio + (mProjectionTargetRatio - mProjectionStartRatio) * blend;

	if (u >= 1.0f)
	{
		mProjectionRatio = mProjectionTargetRatio;
		mbProjectionTransitioning = false;
	}
}

bool FGraphicsManager::HasShowFlag(EEngineShowFlags Flag) const
{
	const uint32 FlagValue = static_cast<uint32>(Flag);
	return (mShowFlags & FlagValue) != 0;
}

void FGraphicsManager::SetShowFlag(EEngineShowFlags Flag, bool bEnable)
{
	const uint32 FlagValue = static_cast<uint32>(Flag);

	if (bEnable)
	{
		mShowFlags |= FlagValue;
	}
	else
	{
		mShowFlags &= ~FlagValue;
	}
}

void FGraphicsManager::SetViewMode(EViewModeIndex InViewMode)
{
	mViewMode = InViewMode;

	switch (mViewMode)
	{
	case EViewModeIndex::VMI_Lit:
		SetWireFrame(false);
		// Lit 렌더링 상태 설정
		break;

	case EViewModeIndex::VMI_Unlit:
		SetWireFrame(false);
		// Unlit 렌더링 상태 설정
		break;

	case EViewModeIndex::VMI_Wireframe:
		SetWireFrame(true);
		break;
	}
}

void FGraphicsManager::CalculateLineBuffer(const TArray<const FRenderInfo*>& renderInfos)
{
	uint32 countIndices = 6 + (mgridExtent / mgridSpacing) * 2 * 2 + renderInfos.Num() * 24;
	uint32 countvertices = 6 + (mgridExtent / mgridSpacing) * 2 + renderInfos.Num() * 8;
	mLineIndices.Reserve(countIndices);
	mLineVertices.Reserve(countvertices);
}
