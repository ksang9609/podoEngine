#pragma once

#include <string_view>

#include "SceneData.h"
#include "TArray.h"
#include "RenderInfo.h"

inline constexpr std::string_view kSceneDataDir = "SceneData\\";
inline constexpr std::string_view kSceneDataSuffix = ".Scene";

class FFileManager;
class UWorld;

class FSceneManager
{
public:
	FSceneManager() = default;
	~FSceneManager() = default;

	// Clear world
	void NewScene();

	void SaveScene(std::string_view sceneName, const FFileManager& fileManager);
	void LoadScene(std::string_view sceneName, const FFileManager& fileManager);

	void Update();
	const TArray<FRenderInfo> GetRenderInfos();

	UWorld* GetCurrentWorld() const { return mCurrentWorld; }

private:
	UWorld* mCurrentWorld = nullptr;
};
