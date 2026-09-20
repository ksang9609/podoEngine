#pragma once

#include "Core/Core.h"

class UWorld;

//inline constexpr std::string_view kSceneDataDir = "SceneData\\";
//inline constexpr std::string_view kSceneDataSuffix = ".Scene";
//FSceneManager 제거 시 FEditorFileUtils로 이동 예정

class FEditorFileUtils
{
public:
	//추후 World 포인터 자체를 교체가능
	static UWorld* LoadScene();

	static bool SaveScene(const UWorld* world);
	static bool SaveSceneAs(const UWorld* world);

	//tk
	//FString saveSceneFileDialog();
	//FString openSceneFileDialog();
	
private:
	static bool saveSceneToPath(const UWorld* world, const FString& filePath);
	static FString openSaveSceneDialog();
	static FString openLoadSceneDialog();

	static FString mCurrentScenePath;
};

