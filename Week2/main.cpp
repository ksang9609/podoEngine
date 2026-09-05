#include <windows.h>
#include "Renderer.h"
#include "Cube.h"
#include "Primitive.h"
#include "FrameTimer.h"
#include "Camera.h"

#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_dx11.h"
#include "imGui/imgui_impl_win32.h"
#include "WindowApplication.h"

#include "Console.h"

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
	WCHAR WindowClass[] = L"JungleWindowClass";
	WCHAR Title[] = L"Game Tech Lab";
	WNDCLASSW wndclass = { 0, WndProc, 0, 0, 0, 0, 0, 0, 0, WindowClass };
	RegisterClassW(&wndclass);

	HWND hWnd = CreateWindowExW(0, WindowClass, Title, WS_POPUP | WS_VISIBLE | WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, 1024, 1024,
		nullptr, nullptr, hInstance, nullptr);

	RAWINPUTDEVICE rid = {};
	rid.usUsagePage = 0x01;		// Generic Desktop
	rid.usUsage = 0x02;			// Mouse
	rid.dwFlags = 0;		// 포커스 있을 때만 수신
	rid.hwndTarget = hWnd;
	RegisterRawInputDevices(&rid, 1, sizeof(rid));

	URenderer renderer;
	renderer.Create(hWnd);
	renderer.CreateShader();
	renderer.CreateConstantBuffer();

	/* Console Window */
	ConsoleWindow& console = ConsoleWindow::GetInstance();
	console.Init("Jungle Console Window", 1024);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	ImGui_ImplWin32_Init((void*)hWnd);
	ImGui_ImplDX11_Init(renderer.Device, renderer.DeviceContext);

	UINT numVerticesCube = sizeof(Cube_vertices) / sizeof(FVertexSimple);
	ID3D11Buffer* vertexBufferCube = renderer.CreateVertexBuffer(Cube_vertices, sizeof(Cube_vertices));

	UFrameTimer FrameTimer(120);

	Sphere* NearCube = new Sphere(FTransform({ -0.2f, -0.2f,  -0.2f  }, { 0, 0, 0 }, { 0.4f, 0.4f, 0.4f }));
	Sphere* FarCube  = new Sphere(FTransform({  0.8f, -0.05f, -0.35f }, { 0, 0, 0 }, { 0.8f, 0.8f, 0.8f }));

	const FVector4 NearTint(1.0f,  0.65f, 0.15f, 0.85f); // 주황 = 가까운 쪽
	const FVector4 FarTint (0.25f, 0.55f, 1.0f,  0.85f); // 파랑 = 먼 쪽

	FCamera* Camera = new FCamera(FTransform({ -2.0f, 1.0f, 1.0f }, { 0, 30, 0 }, { 1, 1, 1 }));
	Camera->LookAt({ 0, 0, 0 });   // NearCube 의 중심
	float aspect = renderer.ViewportInfo.Width / renderer.ViewportInfo.Height;
	float fovDegree = 60.0f;   // 60도
	bool bwireFrame=false;


	// Main Loop
	bool bIsExit = false;
	while (bIsExit == false)
	{
		FrameTimer.StartFrame();
		float deltaTime = FrameTimer.GetDeltaTime();

		ProcessMessage(bIsExit);
		WindowApplication.ProcessDeferredEvents();

		//CameraMove
		{
			const FInputState& Input = WindowApplication.Input;

			// 회전을 이동보다 먼저 — 이번 프레임에 돌린 방향으로 바로 움직이게
			if (!io.WantCaptureMouse && Input.IsDown(VK_RBUTTON))
			{
				Camera->Rotate(Input.MouseDX, Input.MouseDY);
			}

			if (!io.WantCaptureMouse && Input.MouseWheelDelta != 0.0f)
			{
				Camera->Speed *= FMath::Pow(1.2f, Input.MouseWheelDelta);
				Camera->Speed = FMath::Clamp(Camera->Speed, 0.1f, 100.0f);
			}

			if (!io.WantCaptureKeyboard)
			{
				const FMatrix R = FMatrix::Rotate(Camera->Transform.Rotation);
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
					Camera->Velocity = MoveInput * Camera->Speed;
					Camera->Transform.Location += Camera->Velocity * deltaTime;
				}
			}
		}

		renderer.Prepare(bwireFrame);
		renderer.PrepareShader();

		FMatrix View = Camera->GetViewMatrix();
		FMatrix Projection = Camera->GetProjectionMatrix(aspect, fovDegree, 0.1f, 100.0f);
		FMatrix ViewProjection = View * Projection;

		// 깊이 테스트가 켜져 있으면 나중에 그린 FarCube 가 깊이 비교에서 탈락해
		// NearCube(주황)가 앞에 남고, 꺼져 있으면 FarCube(파랑)가 그 위를 덮어쓴다.
		renderer.UpdateConstant(NearCube->Transform.MakeMatrix(), ViewProjection);
		renderer.RenderPrimitive(vertexBufferCube, numVerticesCube);

		renderer.UpdateConstant(FarCube->Transform.MakeMatrix(), ViewProjection);
		renderer.RenderPrimitive(vertexBufferCube, numVerticesCube);

		//ImGui
		{
			ImGui_ImplDX11_NewFrame();
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();
			
			ImGui::Begin("Jungle Property Window");
			ImGui::Text("Hello Jungle World!");
			ImGui::Text("FPS: %.1f  dt: %.4f", FrameTimer.GetFPS(), FrameTimer.GetDeltaTime());

			ImGui::Separator();
			if (ImGui::BeginCombo("##ShowFlags", "Show Flags"))
			{
				ImGui::Checkbox("Wire frame", &bwireFrame);
				ImGui::EndCombo();
			}
			ImGui::Text("FOV     ");
			ImGui::SameLine();
			ImGui::SliderFloat("##FOV", &fovDegree, 0.0f, 180.0f);

			// 1) 라벨 텍스트를 먼저 그리고 같은 줄로
			ImGui::Text("Location");
			ImGui::SameLine();

			// 2) 텍스트를 그린 "뒤"의 남은 폭을 기준으로 계산
			const float spacing = ImGui::GetStyle().ItemSpacing.x;
			const float itemWidth = (ImGui::GetContentRegionAvail().x - spacing * 2.0f) / 3.0f;

			ImGui::SetNextItemWidth(itemWidth);
			ImGui::DragFloat("##CamLocX", &Camera->Transform.Location.x, -10.0f, 10.0f);
			ImGui::SameLine();
			ImGui::SetNextItemWidth(itemWidth);
			ImGui::DragFloat("##CamLocY", &Camera->Transform.Location.y, -10.0f, 10.0f);
			ImGui::SameLine();
			ImGui::SetNextItemWidth(itemWidth);
			ImGui::DragFloat("##CamLocZ", &Camera->Transform.Location.z, -10.0f, 10.0f);

			ImGui::Text("Rotation");
			ImGui::SameLine();
			ImGui::SetNextItemWidth(itemWidth);
			ImGui::DragFloat("##CamRotX", &Camera->Transform.Rotation.Roll, -10.0f, 180.0f);
			ImGui::SameLine();
			ImGui::SetNextItemWidth(itemWidth);
			ImGui::DragFloat("##CamRotY", &Camera->Transform.Rotation.Pitch, -10.0f, 180.0f);
			ImGui::SameLine();
			ImGui::SetNextItemWidth(itemWidth);
			ImGui::DragFloat("##CamRotZ", &Camera->Transform.Rotation.Yaw, -10.0f, 180.0f);
			//ImGui::Checkbox("Depth Test", &renderer.bDepthTestEnabled);
			//ImGui::TextUnformatted(renderer.bDepthTestEnabled
			//	? "ON : orange (near) stays in front"
			//	: "OFF: blue (far, drawn last) overwrites");

			ImGui::End();
			console.Draw();
			ImGui::Render();
			ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
		}

		renderer.SwapBuffer();
		FrameTimer.EndFrame();
	}

	delete(NearCube);
	delete(FarCube);
	delete(Camera);

	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	renderer.ReleaseVertexBuffer(vertexBufferCube);

	renderer.ReleaseConstantBuffer();
	renderer.ReleaseShader();
	renderer.Release();

	return 0;
}
