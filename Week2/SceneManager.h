#pragma once

#include <string_view>

#include "SceneData.h"
#include "TArray.h"
#include "RenderInfo.h"

inline constexpr std::string_view kSceneDataDir = "SceneData\\";
inline constexpr std::string_view kSceneDataSuffix = ".Scene";

class FFileManager;
class FFrameTimer;
class FEditorViewportClient;
class GraphicsManager;
class UWorld;

struct FGuiReference
{
	const FFrameTimer& FrameTimer;
	GraphicsManager* GraphicsManager;
	FEditorViewportClient* ViewportClient;
	const FFileManager* FileManager;
};

struct FGuiInputField
{
	/* Spawn Actor */
	EPrimitive PrimitiveType = EPrimitive::EP_Cube;
	int32 SpawnCount = 1;

	/* Scene Control */
	char SceneName[512] = "Default";

	/* Object Lists */
	TArray<UObject*> SortedObjectLists;
	uint64 LastGUObjectRevision = -1;
};

class FSceneManager
{
public:
	FSceneManager();
	~FSceneManager();

	void Update(float delaTime);
	void UpdateGUI(const FGuiReference& guiReference);

	const TArray<FRenderInfo> GetRenderInfos();

	// Clear world
	void NewScene();
	void DeleteScene();

	void SaveScene(std::string_view sceneName, const FFileManager& fileManager);
	void LoadScene(std::string_view sceneName, const FFileManager& fileManager);

	UWorld* GetCurrentWorld() const { return mCurrentWorld; }

private:
	UWorld* mCurrentWorld = nullptr;
	FGuiInputField mGuiInputField;

	void updateControlPanelGUI(const FGuiReference& guiReference);
	void updatePropertyWindowGUI(const FGuiReference& guiReference);
};
