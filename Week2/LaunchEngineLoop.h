#pragma once

#include <Windows.h>
#include "FrameTimer.h"
#include "Camera.h"
#include "Renderer.h"

#include <d3d11.h>

class Sphere;
class GraphicsManager;

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
	GraphicsManager *GM;
	bool bwireFrame = false;
	bool GInTick = false;
};

inline FEngineLoop GEngineLoop;
