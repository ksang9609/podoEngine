#pragma once

#include <string_view>

#include "SceneData.h"

inline constexpr std::string_view kSceneDataSuffix = ".Scene";

class FFileManager;
class UWorld;

class FSceneManager
{
public:
	FSceneManager() = default;
	~FSceneManager() = default;

	// Clear world
	void NewScene(UWorld* world);

	void SaveScene(std::string_view sceneName, const FFileManager& fileManager, const UWorld* world);
	void LoadScene(std::string_view sceneName, const FFileManager& fileManager, UWorld* world);

private:
	FSceneData ReadSceneData(std::string_view sceneName, const FFileManager& fileManager);
	UWorld* BuildWorldFromSceneData(const FSceneData& sceneData);
};
