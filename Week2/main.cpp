#include <windows.h>

#include "Sphere.h"
#include "Renderer.h"
#include "Cube.h"
#include "Primitive.h"
#include "FrameTimer.h"
#include "Camera.h"

#include "WindowApplication.h"
#include "LaunchEngineLoop.h"

enum : UINT_PTR
{
	RESIZE_TIMER_ID = 1,
};
#include "Console.h"
#include "Object.h"
#include "GraphicsManager.h"

#include "CubeComponent.h"
#include "SphereComponent.h"

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
		PostQuitMessage(0);
		break;

	//다음 메세지들은 입력 지연
	case WM_KEYDOWN: case WM_KEYUP:
	case WM_LBUTTONDOWN: case WM_LBUTTONUP:
	case WM_RBUTTONDOWN: case WM_RBUTTONUP:
	case WM_MOUSEMOVE:   case WM_MOUSEWHEEL:
	case WM_KILLFOCUS:
		WindowApplication.Defer({ hWnd, message, wParam, lParam });
		return 0;

	//마우스가 얼마정도 이동했나
	case WM_INPUT:
	{
		FDeferredMessage M{ hWnd, message, wParam, lParam };

		BYTE  buf[sizeof(RAWINPUT)];
		UINT  size = sizeof(buf);
		if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, buf, &size, sizeof(RAWINPUTHEADER)) != (UINT)-1)
		{
			const RAWINPUT* ri = (const RAWINPUT*)buf;
			if (ri->header.dwType == RIM_TYPEMOUSE &&
				(ri->data.mouse.usFlags & MOUSE_MOVE_ABSOLUTE) == 0)
			{
				M.RawMouseDX = ri->data.mouse.lLastX;
				M.RawMouseDY = ri->data.mouse.lLastY;
			}
		}
		WindowApplication.Defer(M);
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	//SYS_ : Alt가 눌린 상태의 입력
	case WM_SYSKEYDOWN: case WM_SYSKEYUP:
		WindowApplication.Defer({ hWnd, message, wParam, lParam });
		return DefWindowProc(hWnd, message, wParam, lParam);

	//창 크기 변경
	case WM_SIZE:
		if (wParam != SIZE_MINIMIZED)
		{
			WindowApplication.PendingWidth = LOWORD(lParam);
			WindowApplication.PendingHeight = HIWORD(lParam);
			WindowApplication.bPendingResize = true;
		}
		break;

	case WM_ENTERSIZEMOVE:
		SetTimer(hWnd, RESIZE_TIMER_ID, 16, nullptr);
		return 0;

	case WM_EXITSIZEMOVE:
		KillTimer(hWnd, RESIZE_TIMER_ID);
		return 0;

	case WM_TIMER:
		if (wParam == RESIZE_TIMER_ID)
		{
			GEngineLoop.Tick(false);
		}
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

		//WinProc 호출
		DispatchMessage(&msg);

		if (msg.message == WM_QUIT)
		{
			bIsExit = true;
			break;
		}
	}
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	GEngineLoop.Init(hInstance, WndProc);

	HWND hWnd = CreateWindowExW(0, WindowClass, Title, WS_POPUP | WS_VISIBLE | WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, 1024, 1024,
		nullptr, nullptr, hInstance, nullptr);


	GraphicsManager graphicsManager(hWnd);
	URenderer* renderer = graphicsManager.GetRenderer();

	RAWINPUTDEVICE rid = {};
	rid.usUsagePage = 0x01;		// Generic Desktop
	rid.usUsage = 0x02;			// Mouse
	rid.dwFlags = 0;		// 포커스 있을 때만 수신
	rid.hwndTarget = hWnd;
	RegisterRawInputDevices(&rid, 1, sizeof(rid));

	/* Console Window */
	ConsoleWindow& console = ConsoleWindow::GetInstance();
	console.Init("Jungle Console Window", 1024);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	ImGui_ImplWin32_Init((void*)hWnd);
	ImGui_ImplDX11_Init(renderer->Device, renderer->DeviceContext);

	graphicsManager.CreateBuffer(EPrimitive::EP_Cube, Cube_vertices, sizeof(Cube_vertices));
	graphicsManager.CreateBuffer(EPrimitive::EP_Sphere, sphere_vertices, sizeof(sphere_vertices));

	UFrameTimer FrameTimer(120);

	Sphere* NearCube = new Sphere(FTransform({ -0.2f, -0.2f,  -0.2f  }, { 0, 0, 0 }, { 0.4f, 0.4f, 0.4f }));
	Sphere* FarCube  = new Sphere(FTransform({  0.8f, -0.05f, -0.35f }, { 0, 0, 0 }, { 0.8f, 0.8f, 0.8f }));

	const FVector4 NearTint(1.0f,  0.65f, 0.15f, 0.85f); // 주황 = 가까운 쪽
	const FVector4 FarTint (0.25f, 0.55f, 1.0f,  0.85f); // 파랑 = 먼 쪽


	UCubeComponent* nearCube = FObjectFactory::ConstructObject<UCubeComponent>(&graphicsManager);
	nearCube->SetRelativeLocation({ -0.2f, -0.2f,  -0.2f });
	nearCube->SetRelativeRotation({ 0, 0, 0 });
	nearCube->SetRelativeScale3D({ 0.4f, 0.4f, 0.4f });

	bool bWireFrame = false;

	UCubeComponent* farCube = FObjectFactory::ConstructObject<UCubeComponent>(&graphicsManager);
	farCube->SetRelativeLocation({ 0.8f, -0.05f, -0.35f });
	farCube->SetRelativeRotation({ 0, 0, 0 });
	farCube->SetRelativeScale3D({ 0.8f, 0.8f, 0.8f });

	USphereComponent* sphere = FObjectFactory::ConstructObject<USphereComponent>(&graphicsManager);
	sphere->SetRelativeLocation({ 0.0f, 0.0f, 0.0f });
	sphere->SetRelativeRotation({ 0, 0, 0 });
	sphere->SetRelativeScale3D({ 1.0f, 1.0f, 1.0f });

	// Main Loop
	bool bIsExit = false;
	while (bIsExit == false)
	{
		ProcessMessage(bIsExit);
		GEngineLoop.Tick(false);
	}

	GEngineLoop.End();
	return 0;
}
