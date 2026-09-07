#pragma once

#include <string_view>

#include "SceneData.h"
#include "TArray.h"
#include "RenderInfo.h"

inline constexpr std::string_view kSceneDataDir = "SceneData\\";
inline constexpr std::string_view kSceneDataSuffix = ".Scene";

class FFileManager;
class FFrameTimer;
class UWorld;

class FSceneManager
{
public:
	FSceneManager();
	~FSceneManager();

	void Update(float delaTime);
	void UpdateGUI(const FFrameTimer& frameTimer, bool* outbWireFrame);

	const TArray<FRenderInfo> GetRenderInfos();

	// Clear world
	void NewScene();
	void DeleteScene();

	void SaveScene(std::string_view sceneName, const FFileManager& fileManager);
	void LoadScene(std::string_view sceneName, const FFileManager& fileManager);

	UWorld* GetCurrentWorld() const { return mCurrentWorld; }

private:
	UWorld* mCurrentWorld = nullptr;
};
