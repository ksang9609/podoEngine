#include <windows.h>
#include "URenderer.h"
#include "Sphere.h"
#include "Ball.h"
#include "FrameTimer.h"
#include "BallSimulation.h"

#include "ImGui/imgui.h"
#include "ImGui/imgui_internal.h"
#include "ImGui/imgui_impl_dx11.h"
#include "imGui/imgui_impl_win32.h"

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

// ���� �޽����� ó���� �Լ�
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

	// ó���� �޽����� �� �̻� ������ ���� ����
	while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
	{
		// Ű �Է� �޽����� ����
		TranslateMessage(&msg);

		// �޽����� ������ ������ ���ν����� ����, �޽����� ������ ����� WndProc ���� ���޵�
		DispatchMessage(&msg);

		if (msg.message == WM_QUIT)
		{
			bIsExit = true;
			break;
		}
		else if (msg.message == WM_KEYDOWN) // Ű���� ������ ��
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

	// Renderer�� Shader ���� ���Ŀ� ���ؽ� ���۸� ����
	UINT numVerticesSphere = sizeof(sphere_vertices) / sizeof(FVertexSimple);
	ID3D11Buffer* vertexBufferSphere = renderer.CreateVertexBuffer(sphere_vertices, sizeof(sphere_vertices));


	UFrameTimer FrameTimer(120);
	FBallSimulation sim(1000);

	//�� ����(ImGui��)
	int numBalls = 1;

	// Main Loop
	bool bIsExit = false;
	while (bIsExit == false)
	{
		FrameTimer.StartFrame();
		float deltaTime = FrameTimer.GetDeltaTime();

		ProcessMessage(bIsExit);

		//���� ����ŭ ���� ����, �����Ѵ�.
		numBalls = min(sim.GetMaxBallCount(), max(1, numBalls));
		sim.SetBallCount(numBalls);
		sim.Simulate(deltaTime);

		// �غ� �۾�
		renderer.Prepare();
		renderer.PrepareShader();

		for (int i = 0; i < UBall::TotalNumBalls; i++)
		{
			UBall* ball = sim.GetBall(i);

			// offset�� ��� ���۷� ������Ʈ �մϴ�.
			renderer.UpdateConstant(ball->Location, ball->Radius);
			renderer.RenderPrimitive(vertexBufferSphere, numVerticesSphere);
		}

		//ImGui
		{
			ImGui_ImplDX11_NewFrame();
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();

			// ���� ImGui UI ��Ʈ�� �߰��� ImGui::NewFrame()�� ImGui::Render() ������ ���⿡ ��ġ�մϴ�.
			ImGui::Begin("Jungle Property Window");
			ImGui::Text("Hello Jungle World!");

			ImGui::Text("FPS: %.1f  dt: %.4f", FrameTimer.GetFPS(), FrameTimer.GetDeltaTime());
			ImGui::Checkbox("Gravity", &sim.bGravity);
			ImGui::InputInt("Number of Balls", &numBalls);
			ImGui::End();

			ImGui::Render();
			ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
		}

		// ���� ȭ�鿡 �������� ���ۿ� �׸��� �۾��� ���� ���۸� ���� ��ȯ�մϴ�.
		renderer.SwapBuffer();
		FrameTimer.EndFrame();
	}

	// ���⿡�� ImGui �Ҹ�
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	//���ؽ� ���� �Ҹ��� Renderer �Ҹ����� ó���մϴ�.
	renderer.ReleaseVertexBuffer(vertexBufferSphere);

	renderer.ReleaseConstantBuffer();
	renderer.ReleaseShader();
	renderer.Release();

	return 0;
}