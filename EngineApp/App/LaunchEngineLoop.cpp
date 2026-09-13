#include "LaunchEngineLoop.h"

#include <windows.h>

#include "Rendering/Renderer.h"
#include "Platform/WindowApplication.h"
#include "Editor/Console.h"
#include "Rendering/GraphicsManager.h"
#include "Engine/Components/CubeComponent.h"
#include "Engine/Components/SphereComponent.h"
#include "Engine/SceneManager.h"
#include "Core/Object/ObjectFactory.h"
#include "Core/Object/Object.h"
#include "Rendering/Primitives/GizmoArrow.h"
#include "ThirdParty/ImGui/imgui.h"
#include "ThirdParty/ImGui/imgui_impl_dx11.h"
#include "ThirdParty/ImGui/imgui_impl_win32.h"
#include "Engine/Actor.h"
#include "Engine/World.h"
#include "Core/Name.h"

// Primitive vertices definitions
#include "Rendering/Primitives/Cube.h"
#include "Rendering/Primitives/Sphere.h"
#include "Rendering/Primitives/Circle.h"
#include "Rendering/Primitives/Triangle.h"
#include "Rendering/Primitives/Primitives.h"
#include "Rendering/Primitives/TexturedPrimitives.h"

void FEngineLoop::Init(HINSTANCE hInstance, WNDPROC WndProc)
{
	// Initialize window infos
	WCHAR WindowClass[] = L"JungleWindowClass";
	WCHAR Title[] = L"Game Tech Lab";
	WNDCLASSW wndclass = { 0, WndProc, 0, 0, 0, 0, 0, 0, 0, WindowClass };
	RegisterClassW(&wndclass);

	HWND hWnd = CreateWindowExW(
		0,
		WindowClass,
		Title,
		WS_VISIBLE | WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, 600, 1024,
		nullptr, nullptr, hInstance, nullptr
	);

	// 창을 화면 크기에 맞게 최대화하여 표시
	ShowWindow(hWnd, SW_SHOWMAXIMIZED);
	UpdateWindow(hWnd);

	// 최대화된 후의 실제 클라이언트 크기를 구해 콘솔에 전달
	RECT clientRect;
	GetClientRect(hWnd, &clientRect);
	int clientWidth = clientRect.right - clientRect.left;
	int clientHeight = clientRect.bottom - clientRect.top;

	RAWINPUTDEVICE rid = {};
	rid.usUsagePage = 0x01;		// Generic Desktop
	rid.usUsage = 0x02;			// Mouse
	rid.dwFlags = 0;		// 포커스 있을 때만 수신
	rid.hwndTarget = hWnd;
	RegisterRawInputDevices(&rid, 1, sizeof(rid));

	mGraphicsManager = new FGraphicsManager(hWnd);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplWin32_Init((void*)hWnd);
	ImGui_ImplDX11_Init(mGraphicsManager->GetRenderer()->Device, mGraphicsManager->GetRenderer()->DeviceContext);
	ImGui::GetIO().IniFilename = "Config/imgui.ini";

	/* Console Window */
	ConsoleWindow& console = ConsoleWindow::GetInstance();
	console.Init("Jungle Console Window", clientWidth);

	mGraphicsManager->CreateBuffer(EPrimitive::EP_Cube, Cube_vertices, sizeof(Cube_vertices));
	mGraphicsManager->CreateBuffer(EPrimitive::EP_Sphere, Sphere_vertices, sizeof(Sphere_vertices));
	mGraphicsManager->CreateBuffer(EPrimitive::EP_GizmoArrow, GizmoArrow_vertices, sizeof(GizmoArrow_vertices));
	mGraphicsManager->CreateBuffer(EPrimitive::EP_Circle, Circle_vertices, sizeof(Circle_vertices));
	mGraphicsManager->CreateBuffer(EPrimitive::EP_Triangle, Triangle_vertices, sizeof(Triangle_vertices));
	mGraphicsManager->CreateBuffer(EPrimitive::EP_BillboardQuad, Quad_vertices, sizeof(Quad_vertices));

	// 큐브 텍스처 6개로 나눈 버전을 사용하려면
	/*BuildCubeAtlasVertices(CubeTextureVertices);

	mGraphicsManager->CreateTexturedBuffer(EPrimitive::EP_Cube, CubeTextureVertices, sizeof(CubeTextureVertices));*/

	// 예시 텍스쳐 사용용
	const int columns = 4;
	const int rows = 4;
	const int faceCells[6] = { 6, 4, 13, 5, 1, 9 };

	// 24: 인덱스 방식 / 36: 기존 방식
	FVertexTextured atlasVertices[24];

	BuildCubeAtlasVertices(atlasVertices, columns, rows,faceCells);

	mGraphicsManager->CreateTexturedBuffer(EPrimitive::EP_Cube, atlasVertices, sizeof(atlasVertices));

	{
		URenderer* renderer = mGraphicsManager->GetRenderer();

		D3D11_BUFFER_DESC desc = {};
		desc.Usage = D3D11_USAGE_IMMUTABLE;
		desc.ByteWidth = sizeof(CubeTextureIndices);
		desc.BindFlags = D3D11_BIND_INDEX_BUFFER;

		D3D11_SUBRESOURCE_DATA data = {};
		data.pSysMem = CubeTextureIndices;

		renderer->Device->CreateBuffer(&desc, &data, &renderer->CubeIndexBuffer);
	}

	// 구 텍스쳐 uv 매핑
	constexpr std::size_t sphereVertexCount = sizeof(Sphere_vertices) / sizeof(Sphere_vertices[0]);

	FVertexTextured sphereTextureVertices[sphereVertexCount];
	BuildSphereTextureVertices(Sphere_vertices,	sphereTextureVertices);
	mGraphicsManager->CreateTexturedBuffer(EPrimitive::EP_Sphere, sphereTextureVertices, sizeof(sphereTextureVertices));

	// mGraphicsManager->CreateTexturedBuffer(EPrimitive::EP_Cube, CubeTextureVertices, sizeof(CubeTextureVertices));
	mGraphicsManager->CreatePrimitiveTexture(EPrimitive::EP_Cube, L"Assets/Textures/CubeTextureSample.dds");
	mGraphicsManager->CreatePrimitiveTexture(EPrimitive::EP_Sphere, L"Assets/Textures/EarthTexture.dds");


	FrameTimer = new FFrameTimer(120);
	ViewportClient = new FEditorViewportClient(); // Todo: cChange to class

	const FVector4 NearTint(1.0f, 0.65f, 0.15f, 0.85f); // 주황 = 가까운 쪽
	const FVector4 FarTint(0.25f, 0.55f, 1.0f, 0.85f); // 파랑 = 먼 쪽

	mSceneManager = new FSceneManager(ViewportClient->GetCamera());
	mFileManager = new FFileManager();

	mSceneManager->Initialize(*ViewportClient, mGraphicsManager);

	mSceneManager->NewScene();
	// mSceneManager->LoadScene("TestScene", *mFileManager);
	{
		UCubeComponent* cube =
			FObjectFactory::ConstructObject<UCubeComponent>(
				FVector(0.0f, -1.5f, 0.0f),
				FRotator(0.0f, 0.0f, 0.0f),
				FVector(1.0f, 1.0f, 1.0f));

		cube->SetUseTexture(true);

		AActor* actor =
			FObjectFactory::ConstructObject<AActor>();

		actor->AddRootSceneComponent(cube);
		mSceneManager->GetCurrentWorld()->AddActor(actor);
	}

	{
		UCubeComponent* cube =
			FObjectFactory::ConstructObject<UCubeComponent>(
				FVector(0.0f, 1.5f, 0.0f),
				FRotator(0.0f, 0.0f, 0.0f),
				FVector(1.0f, 1.0f, 1.0f));

		cube->SetUseTexture(false);

		AActor* actor =
			FObjectFactory::ConstructObject<AActor>();

		actor->AddRootSceneComponent(cube);
		mSceneManager->GetCurrentWorld()->AddActor(actor);
	}

	{
		USphereComponent* sphere =
			FObjectFactory::ConstructObject<USphereComponent>(
				FVector(0.0f, 1.5f, 0.0f),
				FRotator(0.0f, 0.0f, -90.0f),
				FVector(1.0f, 1.0f, 1.0f));

		// 구의 텍스처 버퍼와 텍스처 셰이더 사용
		sphere->SetUseTexture(true);

		AActor* actor =
			FObjectFactory::ConstructObject<AActor>();

		actor->AddRootSceneComponent(sphere);

		mSceneManager->GetCurrentWorld()->AddActor(actor);
	}
	//test code
	//{
	//	UCubeComponent* cubeComonent = FObjectFactory::ConstructObject<UCubeComponent>(FVector(0), FRotator(), FVector(1));
	//	AActor* cubeActor = FObjectFactory::ConstructObject<AActor>();
	//	cubeActor->AddComponent(cubeComonent);
	//	mSceneManager.GetCurrentWorld()->AddActor(cubeActor);
	//}


	
}

void FEngineLoop::Tick(bool bPumpMessages)
{
	if (GInTick) return;
	GInTick = true;

	FrameTimer->StartFrame();
	float deltaTime = FrameTimer->GetDeltaTime();
	ConsoleWindow& console = ConsoleWindow::GetInstance();

	//Input Threads
	{
		WindowApplication.ProcessDeferredEvents();

		//ImGui Input
		{
			mSceneManager->UpdateGUI({ *FrameTimer, mGraphicsManager, ViewportClient, mFileManager });
		}

		mGraphicsManager->UpdateProjectionTransition(deltaTime);
		ViewportClient->Update(deltaTime, mGraphicsManager->GetRenderer()->ViewportInfo, mSceneManager, mGraphicsManager->GetPerspectiveRatio());
	}

	//Physics Threads
	{

	}

	//Game Threads
	{
		// 레이캐스트보다 먼저 돌려야 한다.
		// 여기서 RenderInfos 가 갱신되고, RayCast 가 그걸 읽는다.
		mSceneManager->Update(deltaTime);
	}

	//Render Threads
	{
		if (WindowApplication.bPendingResize)
		{
			float viewportWidth = mSceneManager->GetPanelWidth();
			float viewportHeight = (1.f - ConsoleWindow::HEIGHT_RATIO) * WindowApplication.PendingHeight;

			mGraphicsManager->GetRenderer()->OnResize(WindowApplication.PendingWidth, WindowApplication.PendingHeight, viewportWidth, viewportHeight);
			WindowApplication.bPendingResize = false;
		}

		mGraphicsManager->Update(deltaTime);

		mGraphicsManager->Render(
			mSceneManager->GetRenderInfos(),
			ViewportClient->mGizmo.GetGizmoRenderInfo(),
			mSceneManager->GetAxisRenderInfos(),
			ViewportClient->GetCamera(),
			mSceneManager->GetSelectedActor()
		);

		//ImGui
		{
			ImGui::Render();
			ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
		}

		// 테스트용 쿼드 그리기
		//mGraphicsManager->GetRenderer()->RenderTestQuad();


		///
		mGraphicsManager->Display();
	}

	FrameTimer->EndFrame();

	GInTick = false;
}

void FEngineLoop::End()
{
	mSceneManager->DeleteScene();

	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	delete FrameTimer;
	delete mSceneManager;
	delete mFileManager;

	delete mGraphicsManager;
}
