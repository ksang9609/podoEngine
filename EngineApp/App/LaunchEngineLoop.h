#pragma once

#include <Windows.h>
#include "Core/FrameTimer.h"
#include "Editor/FEditorViewportClient.h"
#include "Rendering/Camera.h"
#include "Editor/SceneManager.h"
#include "Core/IO/FileManager.h"
#include "Rendering/Renderer.h"
#include "Engine/World.h"

#include <d3d11.h>

class Sphere;
class FGraphicsManager;
class FEngineLoop
{
public:
	FEngineLoop()
	{
	}
	~FEngineLoop() {};

	void Init(HINSTANCE hInstance, WNDPROC WndProc);
	void Tick(bool bPumpMessages);
	void End();
private:
	// Todo: Make as pointer
	FFrameTimer* FrameTimer;
	bool GInTick = false;
	FEditorViewportClient* ViewportClient;

	FGraphicsManager* mGraphicsManager;
	FSceneManager* mSceneManager;
	FFileManager* mFileManager;
};

inline FEngineLoop GEngineLoop;
