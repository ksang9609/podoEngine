#pragma once

#include <Windows.h>
#include "FrameTimer.h"
#include "FEditorViewportClient.h"
#include "Camera.h"
#include "SceneManager.h"
#include "FileManager.h"
#include "Renderer.h"
#include "World.h"

#include <d3d11.h>

class Sphere;
class GraphicsManager;
class FEngineLoop
{
public:
	FEngineLoop()
		: FrameTimer(120)
		, mFileManager()
	{
	}
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
	FSceneManager mSceneManager;
	FFileManager mFileManager;
};

//inline FEngineLoop GEngineLoop;
