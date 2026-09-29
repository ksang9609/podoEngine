#pragma once

#include <optional>

#include "Core/Core.h"
#include "../Rendering/Camera.h"

class UWorld;

//inline constexpr std::string_view kSceneDataDir = "SceneData\\";
//inline constexpr std::string_view kSceneDataSuffix = ".Scene";

struct FLoadedScene
{
    UWorld* World = nullptr;
    std::optional<FCamera> PerspectiveCamera;
};

class FEditorFileUtils
{
public:
	//추후 World 포인터 자체를 교체가능
	//static UWorld* LoadScene();
	static FLoadedScene LoadScene(); // 카메라까지 호출하기 위함

	static bool SaveScene(UWorld* world, FCamera* perspectiveCamera);
	static bool SaveSceneAs(UWorld* world, FCamera* perspectiveCamera);

	static bool ConvertLegacySceneToNewFormat();
	
private:
	static bool saveSceneToPath(UWorld* world, const FString& filePath, FCamera* perspectiveCamera);
	static FString openSaveSceneDialog();
	static FString openLoadSceneDialog();
	static FString openLegacySceneDialog();

	static FString mCurrentScenePath;
};
