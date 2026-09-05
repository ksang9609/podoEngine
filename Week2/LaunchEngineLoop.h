#pragma once

#include <Windows.h>
#include "FrameTimer.h"
#include "Camera.h"
#include "Renderer.h"

#include <d3d11.h>

class Sphere;

class FEngineLoop
{
public:
	FEngineLoop() : FrameTimer(120) {}
	~FEngineLoop() {};

	void Init(HINSTANCE hInstance, WNDPROC WndProc);
	void Tick(bool bPumpMessages);
	void End();

private:
	UFrameTimer FrameTimer;
	FCamera Camera;
	URenderer renderer;
	bool bwireFrame = false;
	float fovDegree = 60.0f;   // 60도
	Sphere* NearCube = nullptr;
	Sphere* FarCube = nullptr;
	UINT numVerticesCube;
	ID3D11Buffer* vertexBufferCube;

	bool GInTick = false;
};

inline FEngineLoop GEngineLoop;
