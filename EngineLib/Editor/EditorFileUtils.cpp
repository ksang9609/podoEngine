#include "EditorFileUtils.h"

#include "Core/Object/ObjectFactory.h"
#include "Core/IO/FileManager.h"
#include "Editor/Console.h"

#include "Engine/EngineStatics.h"
#include "Engine/World.h"
#include "Engine/Serialization/JsonArchive.h"

#include <filesystem>
#include <Windows.h>
#include <commdlg.h>

FString FEditorFileUtils::mCurrentScenePath = "";

bool FEditorFileUtils::SaveScene(const UWorld* world, FFileManager& fileManager)
{
	if (!world)
	{
		UE_LOG_F(Error, Core, "SaveScene failed: World is null");
		return false;
	}

	if (mCurrentScenePath.IsEmpty())
	{
		UE_LOG_F(Log, Core, "SaveScene: No current scene path. Redirecting to Save As");
		return SaveSceneAs(world, fileManager);
	}

	//Path = CurrentScenePath;
	if (!saveSceneToPath(world, mCurrentScenePath, fileManager))
	{
		return false;
	}

	UE_LOG_F(Log, Core, "Scene saved: {}", mCurrentScenePath.CStr());
	return true;
}

bool FEditorFileUtils::SaveSceneAs(const UWorld* world, FFileManager& fileManager)
{
	if (!world)
	{
		UE_LOG_F(Error, Core, "SaveSceneAs failed: World is null");
		return false;
	}

	FString filePath = openSaveSceneDialog();

	if (filePath.IsEmpty())
	{
		UE_LOG_F(Log, Core, "SaveSceneAs canceled");
		return false;
	}

	std::filesystem::path normalizedPath = std::filesystem::absolute(filePath.CStr()).lexically_normal();
	FString normalizedScenePath(normalizedPath.string());

	if (!saveSceneToPath(world, normalizedScenePath, fileManager))
	{
		UE_LOG_F(Error, Core, "SaveSceneAs failed: {}", normalizedPath.string().c_str());
		return false;
	}

	mCurrentScenePath = normalizedPath.string();

	UE_LOG_F(Log, Core, "Scene saved as: {}", mCurrentScenePath.CStr());

	return true;
}

bool FEditorFileUtils::saveSceneToPath(const UWorld* world, const FString& filePath, FFileManager& fileManager)
{
	try
	{
		json::JSON worldJson = json::JSON::Make(json::JSON::Class::Object);
		world->SerializeClass(worldJson);

		json::JSON sceneJson = FJsonArchive::Serialize(worldJson, UEngineStatics::GetNextUUID());
	
		//FString sceneJsonText(sceneJson.dump()); // 한줄로 저장
		FString sceneJsonText(sceneJson.dump(1, "  "));

		fileManager.WriteStringToFile(filePath, sceneJsonText);
	}
	catch (const std::exception& e)
	{
		UE_LOG_F(Error, Core, "Scene write failed: {}", e.what());
		return false;
	}

	return true;
}

UWorld* FEditorFileUtils::LoadScene(FFileManager& fileManager)
{
	FString filePath = openLoadSceneDialog();

	if (filePath.IsEmpty())
	{
		UE_LOG_F(Log, Core, "LoadScene canceled");
		return nullptr;
	}

	std::filesystem::path normalizedPath = std::filesystem::absolute(filePath.CStr()).lexically_normal();
	FString normalizedScenePath(normalizedPath.string());

	try
	{
		FString sceneJsonText = fileManager.ReadFileToString(normalizedPath.string());

		json::JSON sceneJson = json::JSON::Load(sceneJsonText);

		FJsonArchiveData archive = FJsonArchive::Deserialize(sceneJson);
		const json::JSON& worldJson = archive.WorldJson;

		if (!worldJson.hasKey("ClassName")
			|| worldJson.at("ClassName").JSONType() != json::JSON::Class::String
			|| FString(worldJson.at("ClassName").ToString()) != UWorld::GetClass()->Name)
		{
			throw std::runtime_error(
				"Scene object must have class UWorld");
		}

		std::unique_ptr<UWorld> newWorld(FObjectFactory::ConstructUnInitializedObject<UWorld>());

		if (!newWorld)
		{
			throw std::runtime_error("Failed to create UWorld");
		}

		newWorld->DeserializeClass(worldJson);

		UEngineStatics::SetNextUUID(archive.NextUUID);
		mCurrentScenePath = normalizedScenePath;

		UE_LOG_F(Log, Core, "Scene loaded: {}", mCurrentScenePath);

		return newWorld.release();
	}
	catch (const std::exception& e)
	{
		UE_LOG_F(Error, Core, "LoadScene failed: {} - {}", e.what(), mCurrentScenePath);

		return nullptr;
	}
}

FString FEditorFileUtils::openSaveSceneDialog()
{
	char filePath[MAX_PATH] = {};

	// OPENFILENAME 설정
	OPENFILENAMEA openFileName = {};
	openFileName.lStructSize = sizeof(OPENFILENAMEA);
	openFileName.hwndOwner = nullptr;
	openFileName.lpstrFile = filePath;
	openFileName.nMaxFile = MAX_PATH;

	openFileName.lpstrFilter = "Scene Files (*.Scene)\0*.Scene\0" "All Files (*.*)\0*.*\0";
	openFileName.lpstrDefExt = "Scene";
	openFileName.lpstrInitialDir = "EngineLib/Assets/SceneData/";
	openFileName.nFilterIndex = 1;

	openFileName.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;


	// FileName 반환
	if (!GetSaveFileNameA(&openFileName))
	{
		return FString("");
	}

	return FString(filePath);
}

FString FEditorFileUtils::openLoadSceneDialog()
{
	char filePath[MAX_PATH] = {};

	// OPENFILENAME 설정
	OPENFILENAMEA openFileName = {};
	openFileName.lStructSize = sizeof(OPENFILENAMEA);
	openFileName.hwndOwner = nullptr;
	openFileName.lpstrFile = filePath;
	openFileName.nMaxFile = MAX_PATH;

	openFileName.lpstrFilter = "Scene Files (*.Scene)\0*.Scene\0" "All Files (*.*)\0*.*\0";
	openFileName.lpstrDefExt = "Scene";
	openFileName.lpstrInitialDir = "SceneData/";
	openFileName.nFilterIndex = 1;

	openFileName.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;


	// FileName 반환
	if (!GetOpenFileNameA(&openFileName))
	{
		return FString("");
	}

	return FString(filePath);
}
