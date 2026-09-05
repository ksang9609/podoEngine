#include <windows.h>

#include "URenderer.h"
#include "Sphere.h"
#include "Cube.h"
#include "Primitive.h"
#include "FrameTimer.h"
#include "Camera.h"

#include "ImGui/imgui.h"
#include "ImGui/imgui_internal.h"
#include "ImGui/imgui_impl_dx11.h"
#include "imGui/imgui_impl_win32.h"

#include "Console.h"
#include "Object.h"
#include "GraphicsManager.h"

#include "CubeComponent.h"

void* operator new(size_t size);
void operator delete(void* deleteObject, size_t size);

static uint32 sTotalAllocationCount;
static uint32 sTotalAllocationBytes;

void* operator new(size_t size)
{
	++sTotalAllocationCount;
	sTotalAllocationBytes += static_cast<uint32>(size);

	void* newObject = malloc(size);

	return newObject;
}

void operator delete(void* deleteObject, size_t size)
{
	assert(deleteObject);

	--sTotalAllocationCount;
	sTotalAllocationBytes -= static_cast<uint32>(size);

	free(deleteObject);
}

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
	{
		return true;
	}

	switch (message)
	{
	case WM_DESTROY:
		// Signal that the app should quit
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}

void ProcessMessage(bool& bIsExit)
{
	MSG msg;

	while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);

		DispatchMessage(&msg);

		if (msg.message == WM_QUIT)
		{
			bIsExit = true;
			break;
		}
		else if (msg.message == WM_KEYDOWN)
		{
		}
	}
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	WCHAR WindowClass[] = L"JungleWindowClass";
	WCHAR Title[] = L"Game Tech Lab";
	WNDCLASSW wndclass = { 0, WndProc, 0, 0, 0, 0, 0, 0, 0, WindowClass };
	RegisterClassW(&wndclass);

	HWND hWnd = CreateWindowExW(0, WindowClass, Title, WS_POPUP | WS_VISIBLE | WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, 1024, 1024,
		nullptr, nullptr, hInstance, nullptr);

	GraphicsManager graphicsManager(hWnd);
	URenderer* renderer = graphicsManager.GetRenderer();

	/* Console Window */
	ConsoleWindow& console = ConsoleWindow::GetInstance();
	console.Init("Jungle Console Window", 1024);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	ImGui_ImplWin32_Init((void*)hWnd);
	ImGui_ImplDX11_Init(renderer->Device, renderer->DeviceContext);

	graphicsManager.CreateBuffer(EPrimitive::EP_Cube, Cube_vertices, sizeof(Cube_vertices));

	UFrameTimer FrameTimer(120);

	// 깊이 테스트 확인용 배치.
	// 카메라가 원점을 향해 +X(언리얼 전방)로 바라보고, 큐브 둘을 그 시선 축 위에 앞뒤로 겹쳐 둔다.
	// Cube_vertices 는 원점이 '최소 코너'라서, 중심을 맞추려면 Location 에 -Scale/2 를 준다.
	//   NearCube : 중심 (0, 0, 0)          — 카메라로부터 2.0
	//   FarCube  : 중심 (1.2, 0.35, 0.05)  — 카메라로부터 약 3.2. 더 크고 오른쪽으로 밀어서
	//              화면상 NearCube 와 절반쯤 겹치게 했다.
	Sphere* NearCube = new Sphere(FTransform({ -0.2f, -0.2f,  -0.2f  }, { 0, 0, 0 }, { 0.4f, 0.4f, 0.4f }));
	Sphere* FarCube  = new Sphere(FTransform({  0.8f, -0.05f, -0.35f }, { 0, 0, 0 }, { 0.8f, 0.8f, 0.8f }));

	// 둘 다 같은 정점 버퍼를 쓰고 카메라를 향한 -X 면이 똑같이 파랑이라, 색으로 구분해 준다.
	const FVector4 NearTint(1.0f,  0.65f, 0.15f, 0.85f); // 주황 = 가까운 쪽
	const FVector4 FarTint (0.25f, 0.55f, 1.0f,  0.85f); // 파랑 = 먼 쪽

	UCubeComponent* nearCube = new UCubeComponent(&graphicsManager);
	nearCube->SetRelativeLocation({ -0.2f, -0.2f,  -0.2f });
	nearCube->SetRelativeRotation({ 0, 0, 0 });
	nearCube->SetRelativeScale3D({ 0.4f, 0.4f, 0.4f });

	UCubeComponent* farCube = new UCubeComponent(&graphicsManager);
	farCube->SetRelativeLocation({ 0.8f, -0.05f, -0.35f });
	farCube->SetRelativeRotation({ 0, 0, 0 });
	farCube->SetRelativeScale3D({ 0.8f, 0.8f, 0.8f });

	// Main Loop
	bool bIsExit = false;
	while (bIsExit == false)
	{
		FrameTimer.StartFrame();
		float deltaTime = FrameTimer.GetDeltaTime();

		ProcessMessage(bIsExit);

		// Update
		{
		}

		// GraphicsManager.Render()
		{
			graphicsManager.Prepare();
			nearCube->Render();
			farCube->Render();

			//ImGui
			{
				ImGui_ImplDX11_NewFrame();
				ImGui_ImplWin32_NewFrame();
				ImGui::NewFrame();

				ImGui::Begin("Jungle Property Window");
				//	ImGui::Text("Hello Jungle World!");

				ImGui::Text("FPS: %.1f  dt: %.4f", FrameTimer.GetFPS(), FrameTimer.GetDeltaTime());

				ImGui::Separator();
				//ImGui::Checkbox("Depth Test", &renderer.bDepthTestEnabled);
				//ImGui::TextUnformatted(renderer.bDepthTestEnabled
				//	? "ON : orange (near) stays in front"
				//	: "OFF: blue (far, drawn last) overwrites");

				ImGui::End();
				console.Draw();
				ImGui::Render();
				ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
			}

			graphicsManager.Display();
		}
		
		FrameTimer.EndFrame();
	}

	delete(NearCube);
	delete(FarCube);

	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	return 0;
}
