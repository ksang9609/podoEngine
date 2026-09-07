#include "LaunchEngineLoop.h"

#include <windows.h>

#include "Renderer.h"
#include "WindowApplication.h"
#include "Console.h"
#include "GraphicsManager.h"
#include "CubeComponent.h"
#include "ObjectFactory.h"
#include "Cube.h"
#include "Sphere.h"
#include "Object.h"

#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_dx11.h"
#include "imGui/imgui_impl_win32.h"
#include "Actor.h"
#include "World.h"


void FEngineLoop::Init(HINSTANCE hInstance, WNDPROC WndProc)
{
	WCHAR WindowClass[] = L"JungleWindowClass";
	WCHAR Title[] = L"Game Tech Lab";
	WNDCLASSW wndclass = { 0, WndProc, 0, 0, 0, 0, 0, 0, 0, WindowClass };
	RegisterClassW(&wndclass);

	HWND hWnd = CreateWindowExW(0, WindowClass, Title, WS_VISIBLE | WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, 1024, 1024,
		nullptr, nullptr, hInstance, nullptr);

	RAWINPUTDEVICE rid = {};
	rid.usUsagePage = 0x01;		// Generic Desktop
	rid.usUsage = 0x02;			// Mouse
	rid.dwFlags = 0;		// 포커스 있을 때만 수신
	rid.hwndTarget = hWnd;
	RegisterRawInputDevices(&rid, 1, sizeof(rid));

	mGraphicsManager = new GraphicsManager(hWnd);

	/* Console Window */
	ConsoleWindow& console = ConsoleWindow::GetInstance();
	console.Init("Jungle Console Window", 1024);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplWin32_Init((void*)hWnd);
	ImGui_ImplDX11_Init(mGraphicsManager->GetRenderer()->Device, mGraphicsManager->GetRenderer()->DeviceContext);

	mGraphicsManager->CreateBuffer(EPrimitive::EP_Cube, Cube_vertices, sizeof(Cube_vertices));
	mGraphicsManager->CreateBuffer(EPrimitive::EP_Sphere, Sphere_vertices, sizeof(Sphere_vertices));

	FrameTimer = new FFrameTimer(120);
	ViewportClient = new FEditorViewportClient(); // Todo: cChange to class

	const FVector4 NearTint(1.0f, 0.65f, 0.15f, 0.85f); // 주황 = 가까운 쪽
	const FVector4 FarTint(0.25f, 0.55f, 1.0f, 0.85f); // 파랑 = 먼 쪽

	mSceneManager = new FSceneManager();
	mFileManager = new FFileManager();


	mSceneManager->NewScene();
	mSceneManager->LoadScene("TestScene", *mFileManager);

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
			mSceneManager->UpdateGUI({ *FrameTimer, mGraphicsManager, ViewportClient });
		}

		ViewportClient->Update(deltaTime);
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

	//Raycast
	{
		//Gizmo Test
		ViewportClient->mGizmo.mbVisible = true;
		ViewportClient->mGizmo.mLocation = { 0.0f, 2.0f, 0.0f };

		ViewportClient->Update(deltaTime);
		ViewportClient->RayCast(mGraphicsManager->GetRenderer()->ViewportInfo, mSceneManager->GetCurrentWorld());

		//UE_LOG("Gizmo axis : %d", static_cast<int>(ViewportClient.mGizmo.eAxis));

		const FInputState& Input = WindowApplication.Input;

		if (ViewportClient->ClickedActor)
		{
			ViewportClient->ClickedActor->BeginFrame();
		}

		// 누른 순간에만 선택을 갱신한다. 떼는 것으로는 선택이 풀리지 않는다.
		if (!ImGui::GetIO().WantCaptureMouse && Input.WasPressed(VK_LBUTTON))
		{
			AActor* Hit = nullptr;

			if (ViewportClient->IsMouseHit())
			{
				UObject* ClickedObject = UObject::GUObjectArray[ViewportClient->HoveredRenderInfo.ObejctID.InternalIndex];
				if (ClickedObject && ClickedObject->IsA(AActor::GetClass()))
				{
					Hit = static_cast<AActor*>(ClickedObject);
				}
			}

			// 다른 것을 눌렀으면 이전 선택 해제. 같은 것이면 유지.
			if (ViewportClient->ClickedActor && ViewportClient->ClickedActor != Hit)
			{
				ViewportClient->ClickedActor->UnPressed();
			}

			ViewportClient->ClickedActor = Hit;   // 빈 공간을 눌렀으면 nullptr -> 선택 해제

			if (Hit)
			{
				Hit->Pressed();      // 선택 유지
				Hit->ClickStart();   // 이번 프레임에 시작했음을 표시
			}
		}
	}

	//Render Threads
	{
		if (WindowApplication.bPendingResize)
		{
			mGraphicsManager->GetRenderer()->OnResize(WindowApplication.PendingWidth, WindowApplication.PendingHeight);
			WindowApplication.bPendingResize = false;
		}

		mGraphicsManager->Update(deltaTime);
		mGraphicsManager->Prepare(&ViewportClient->mCamera);
		mGraphicsManager->Render(mSceneManager->GetRenderInfos());

		//강조
		if (ViewportClient->IsMouseHit())
		{
			//UE_LOG("Hit");

			//큐브가 선택되었으면 강조 표시
			mGraphicsManager->RenderHighLight(ViewportClient->HoveredRenderInfo);
		}

		//ImGui
		{
			ImGui::Render();
			ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
		}

		mGraphicsManager->RenderOverlay(ViewportClient->mGizmo.GetGizmoRenderInfo());
		mGraphicsManager->Display();
	}

	FrameTimer->EndFrame();

	GInTick = false;
}

void FEngineLoop::End()
{
	// Debug
	mSceneManager->SaveScene("TestScene", *mFileManager);
	mSceneManager->DeleteScene();

	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	delete FrameTimer;
	delete mSceneManager;
	delete mFileManager;

	delete mGraphicsManager;
}
