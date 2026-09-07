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

#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_dx11.h"
#include "imGui/imgui_impl_win32.h"
#include "Actor.h"
#include "World.h"
#include "Console.h"

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

	UFrameTimer FrameTimer(120);

	const FVector4 NearTint(1.0f, 0.65f, 0.15f, 0.85f); // 주황 = 가까운 쪽
	const FVector4 FarTint(0.25f, 0.55f, 1.0f, 0.85f); // 파랑 = 먼 쪽

	bwireFrame = false;

	mSceneManager.NewScene();
	mSceneManager.LoadScene("TestScene", mFileManager);

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

	FrameTimer.StartFrame();
	float deltaTime = FrameTimer.GetDeltaTime();
	ConsoleWindow& console = ConsoleWindow::GetInstance();

	//Input Threads
	{
		WindowApplication.ProcessDeferredEvents();

		//ImGui Input
		{
			ImGui_ImplDX11_NewFrame();
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();

			ImGui::Begin("Jungle Property Window");
			ImGui::Text("Hello Jungle World!");
			ImGui::Text("FPS: %.1f  dt: %.4f", FrameTimer.GetFPS(), FrameTimer.GetDeltaTime());

			ImGui::Separator();
			//ImGui::SliderFloat("Speed", &Camera.Speed, -10.0f, 10.0f);
			if (ImGui::BeginCombo("##ShowFlags", "Show Flags"))
			{
				ImGui::Checkbox("Wire frame", &bwireFrame);
				ImGui::EndCombo();
			}
			ImGui::Text("FOV     ");
			ImGui::SameLine();
			//ImGui::SliderFloat("##FOV", &fovDegree, 0.0f, 180.0f);

			// 1) 라벨 텍스트를 먼저 그리고 같은 줄로
			ImGui::Text("Location");
			ImGui::SameLine();

			// 2) 텍스트를 그린 "뒤"의 남은 폭을 기준으로 계산
			const float spacing = ImGui::GetStyle().ItemSpacing.x;
			const float itemWidth = (ImGui::GetContentRegionAvail().x - spacing * 2.0f) / 3.0f;

			//ImGui::SetNextItemWidth(itemWidth);
			//ImGui::DragFloat("##CamLocX", &Camera.Transform.Location.x, -10.0f, 10.0f);d
			//ImGui::SameLine();
			//ImGui::SetNextItemWidth(itemWidth);
			//ImGui::DragFloat("##CamLocY", &Camera.Transform.Location.y, -10.0f, 10.0f);
			//ImGui::SameLine();
			//ImGui::SetNextItemWidth(itemWidth);
			//ImGui::DragFloat("##CamLocZ", &Camera.Transform.Location.z, -10.0f, 10.0f);

			//ImGui::Text("Rotation");
			//ImGui::SameLine();
			//ImGui::SetNextItemWidth(itemWidth);
			//ImGui::DragFloat("##CamRotX", &Camera.Transform.Rotation.Roll, -10.0f, 180.0f);
			//ImGui::SameLine();
			//ImGui::SetNextItemWidth(itemWidth);
			//ImGui::DragFloat("##CamRotY", &Camera.Transform.Rotation.Pitch, -10.0f, 180.0f);
			//ImGui::SameLine();
			//ImGui::SetNextItemWidth(itemWidth);
			//ImGui::DragFloat("##CamRotZ", &Camera.Transform.Rotation.Yaw, -10.0f, 180.0f);
			//ImGui::Checkbox("Depth Test", &renderer.bDepthTestEnabled);
			//ImGui::TextUnformatted(renderer.bDepthTestEnabled
			//	? "ON : orange (near) stays in front"
			//	: "OFF: blue (far, drawn last) overwrites");

			ImGui::End();
		}

		//Physics Threads
		{

		}

		//Raycast
		{
			ViewportClient.Update(deltaTime);
			ViewportClient.RayCast(mGraphicsManager->GetRenderer()->ViewportInfo, mSceneManager.GetCurrentWorld());

			if (ViewportClient.IsMouseHit() && ViewportClient.ClickedActor)
			{
				ViewportClient.ClickedActor->Clicked();
			}
			else if (ViewportClient.ClickedActor)
			{
				ViewportClient.ClickedActor->UnClicked();
				ViewportClient.ClickedActor = nullptr;
			}
		}

		//Game Threads
		{
			mSceneManager.Update();
		}


		//Render Threads
		{

			if (WindowApplication.bPendingResize)
			{
				mGraphicsManager->GetRenderer()->OnResize(WindowApplication.PendingWidth, WindowApplication.PendingHeight);
				WindowApplication.bPendingResize = false;
			}

			mGraphicsManager->Update(deltaTime);
			mGraphicsManager->Prepare(bwireFrame, &ViewportClient.mCamera);
			mGraphicsManager->Render(mSceneManager.GetRenderInfos());


			//ImGui
			{
				console.Draw();
				ImGui::Render();
				ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
			}

			mGraphicsManager->Display();

			//강조
			if (ViewportClient.IsMouseHit())
			{
				UE_LOG("Hit");

				//큐브가 선택되었으면 강조 표시
				//mGraphicsManager->GetRenderer()->DeviceContext->OMSetRenderTargets(0, nullptr, D)
			}
		}

		FrameTimer.EndFrame();

		GInTick = false;
	}
}

void FEngineLoop::End()
{

	// Debug
	mSceneManager.SaveScene("TestScene", mFileManager);

	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	delete mGraphicsManager;
}
