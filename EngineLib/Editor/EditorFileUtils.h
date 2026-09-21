#pragma once

#include "Core/Core.h"

class UWorld;
class FFileManager;

//inline constexpr std::string_view kSceneDataDir = "SceneData\\";
//inline constexpr std::string_view kSceneDataSuffix = ".Scene";

class FEditorFileUtils
{
public:
	//추후 World 포인터 자체를 교체가능
	static UWorld* LoadScene(FFileManager& fileManager);

	static bool SaveScene(const UWorld* world, FFileManager& fileManager);
	static bool SaveSceneAs(const UWorld* world, FFileManager& fileManager);
	
private:
	static bool saveSceneToPath(const UWorld* world	
		, const FString& filePath
		, FFileManager& fileManager
	);

	static FString openSaveSceneDialog();
	static FString openLoadSceneDialog();

	static FString mCurrentScenePath;
};

