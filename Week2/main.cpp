#include <windows.h>
#include "URenderer.h"
#include "Sphere.h"
#include "Ball.h"
#include "FrameTimer.h"
#include "Camera.h"

#include "ImGui/imgui.h"
#include "ImGui/imgui_internal.h"
#include "ImGui/imgui_impl_dx11.h"
#include "imGui/imgui_impl_win32.h"

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

	URenderer renderer;
	renderer.Create(hWnd);
	renderer.CreateShader();
	renderer.CreateConstantBuffer();

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	ImGui_ImplWin32_Init((void*)hWnd);
	ImGui_ImplDX11_Init(renderer.Device, renderer.DeviceContext);

	UINT numVerticesSphere = sizeof(sphere_vertices) / sizeof(FVertexSimple);
	ID3D11Buffer* vertexBufferSphere = renderer.CreateVertexBuffer(sphere_vertices, sizeof(sphere_vertices));

	UFrameTimer FrameTimer(120);
	UBall* ball = new UBall(FTransform({-0.5,0,0}, {0, 0, 0}, {0.1, 0.5, 0.1}), FVector(0));
	FCamera* Camera = new FCamera(FTransform({ 0.5, 0.5, 0.5 }, {0, 0, 0}, {1, 1, 1}));

	// Main Loop
	bool bIsExit = false;
	while (bIsExit == false)
	{
		FrameTimer.StartFrame();
		float deltaTime = FrameTimer.GetDeltaTime();

		ProcessMessage(bIsExit);

		renderer.Prepare();
		renderer.PrepareShader();

		//1. Local -> World
		FMatrix World = ball->Transform.MakeMatrix();

		//2. World -> View
		FVector Forward = ball->Transform.Location - Camera->Transform.Location;
		Forward.Normalize();
		FVector Right = FVector::cross(FVector(0, 1, 0), Forward);
		Right.Normalize();
		FVector Up = FVector::cross(Forward, Right);

		FMatrix View = FMatrix::Translation(-Camera->Transform.Location) * FMatrix::Rotate(Camera->Transform.Rotation).Transpose();

		renderer.UpdateConstant(World, View);
		renderer.RenderPrimitive(vertexBufferSphere, numVerticesSphere);

		//ImGui
		{
			ImGui_ImplDX11_NewFrame();
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();

			ImGui::Begin("Jungle Property Window");
			ImGui::Text("Hello Jungle World!");

			ImGui::Text("FPS: %.1f  dt: %.4f", FrameTimer.GetFPS(), FrameTimer.GetDeltaTime());
			ImGui::End();

			ImGui::Render();
			ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
		}

		renderer.SwapBuffer();
		FrameTimer.EndFrame();
	}

	delete(ball);

	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	renderer.ReleaseVertexBuffer(vertexBufferSphere);

	renderer.ReleaseConstantBuffer();
	renderer.ReleaseShader();
	renderer.Release();

	return 0;
}
