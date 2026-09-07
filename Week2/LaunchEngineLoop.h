#pragma once

#include <Windows.h>
#include "FrameTimer.h"
#include "FEditorViewportClient.h"
#include "Camera.h"
#include "Renderer.h"
#include "World.h"

#include <d3d11.h>

class Sphere;
class GraphicsManager;

struct FObjectID
{
	int32 UUID;
	int32 InternalIndex;
};

class FEngineLoop
{
public:
	FEngineLoop() : FrameTimer(120) {}
	~FEngineLoop() {};

	void Init(HINSTANCE hInstance, WNDPROC WndProc);
	void Tick(bool bPumpMessages);
	void End();

private:
	// Todo: Make as pointer
	UFrameTimer FrameTimer;
	bool bwireFrame = false;
	bool GInTick = false;
	FEditorViewportClient ViewportClient;
	GraphicsManager* mGraphicsManager;
	UWorld* mWorld;
};

//inline FEngineLoop GEngineLoop;
