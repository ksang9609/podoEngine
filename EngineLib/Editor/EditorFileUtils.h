#pragma once

#include "Core/Core.h"

class UWorld;

//inline constexpr std::string_view kSceneDataDir = "SceneData\\";
//inline constexpr std::string_view kSceneDataSuffix = ".Scene";

class FEditorFileUtils
{
public:
	//추후 World 포인터 자체를 교체가능
	static UWorld* LoadScene();

	static bool SaveScene(const UWorld* world);
	static bool SaveSceneAs(const UWorld* world);
	
private:
	static bool saveSceneToPath(const UWorld* world, const FString& filePath);
	static FString openSaveSceneDialog();
	static FString openLoadSceneDialog();

	static FString mCurrentScenePath;
};

