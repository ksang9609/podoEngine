#include "GraphicsManager.h"

#include "Renderer.h"
#include "WindowApplication.h"
#include "ImGui/imgui.h"

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
	mFovDegree = 60.f;
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

void GraphicsManager::Prepare(bool bWireFrame)
{
	mRenderer->Prepare(bWireFrame);
	mRenderer->PrepareShader();

	// Todo: 
	FMatrix view = mCamera->GetViewMatrix();
	FMatrix projection = mCamera->GetProjectionMatrix(mAspect, mFovDegree, 0.1f, 100.0f);
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

		FBuffer vertexBuffer = mBufferMap[renderInfo.ePrimitive];
		mRenderer->RenderPrimitive(vertexBuffer.Buffer, vertexBuffer.SourceNum);
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
	ImGuiIO& io = ImGui::GetIO();

	mAspect = mRenderer->ViewportInfo.Width / mRenderer->ViewportInfo.Height;
	//CameraMove
	const FInputState& Input = WindowApplication.Input;

	// 회전을 이동보다 먼저 — 이번 프레임에 돌린 방향으로 바로 움직이게
	if (!io.WantCaptureMouse && Input.IsDown(VK_RBUTTON))
	{
		mCamera->Rotate(Input.MouseDX, Input.MouseDY);
	}

	if (!io.WantCaptureMouse && Input.MouseWheelDelta != 0.0f)
	{
		mCamera->Speed *= FMath::Pow(1.2f, Input.MouseWheelDelta);
		mCamera->Speed = FMath::Clamp(mCamera->Speed, 0.1f, 100.0f);
	}

	if (!io.WantCaptureKeyboard)
	{
		const FMatrix R = FMatrix::Rotate(mCamera->Transform.Rotation);
		const FVector Forward = R.GetUnitAxis(EAxis::X);
		const FVector Right = R.GetUnitAxis(EAxis::Y);

		FVector MoveInput(0.f, 0.f, 0.f);
		if (Input.IsDown('W')) MoveInput += Forward;
		if (Input.IsDown('S')) MoveInput -= Forward;
		if (Input.IsDown('D')) MoveInput += Right;
		if (Input.IsDown('A')) MoveInput -= Right;
		if (Input.IsDown('E')) MoveInput += FVector(0.f, 0.f, 1.f);   // 상승은 월드 업 기준
		if (Input.IsDown('Q')) MoveInput -= FVector(0.f, 0.f, 1.f);

		if (MoveInput.Length() > SMALL_NUMBER)
		{
			MoveInput.Normalize();
			mCamera->Velocity = MoveInput * mCamera->Speed;
			mCamera->Transform.Location += mCamera->Velocity * deltaTime;
		}
	}
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
