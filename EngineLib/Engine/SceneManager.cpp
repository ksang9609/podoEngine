
#include "SceneManager.h"

#include <algorithm>
#include <format>

#include "Core/Container/TArray.h"
#include "Core/IO/FileManager.h"
#include "Core/IO/JsonUtil.h"
#include "Core/Object/ObjectFactory.h"
#include "Core/enum.h"
#include "Editor/Console.h"
#include "Editor/FEditorViewportClient.h"
#include "Engine/Components/PrimitiveComponent.h"
#include "Engine/EngineStatics.h"
#include "Engine/World.h"
#include "Rendering/Camera.h"

#include "ThirdParty/ImGui/imgui.h"
#include "ThirdParty/ImGui/imgui_impl_dx11.h"
#include "ThirdParty/ImGui/imgui_impl_win32.h"

#include "Core/FrameTimer.h"
#include "Engine/Components/ActorComponent.h"
#include "Engine/Components/CubeComponent.h"

FSceneManager::FSceneManager(const FCamera& viewportCameraRef)
	: mViewportCameraRef(viewportCameraRef)
{
}

FSceneManager::~FSceneManager()
{
	delete mCurrentWorld;
}

void FSceneManager::Update(float delaTime)
{
	// Todo: Save / Load
	{

	}

	mCurrentWorld->Update();
}

void FSceneManager::NewScene()
{
	if (mCurrentWorld != nullptr)
	{
		delete mCurrentWorld;
	}

	UEngineStatics::SetNextUUID(0);
	ResetSelectedActor();
	mCurrentWorld = FObjectFactory::ConstructObject<UWorld>();
}

void FSceneManager::DeleteScene()
{
	if (mCurrentWorld != nullptr)
	{
		delete mCurrentWorld;
		mCurrentWorld = nullptr;
	}
	ResetSelectedActor();
}

void FSceneManager::SaveScene(
	std::string_view sceneName,
	const FFileManager& fileManager)
{
	FString fileName = kSceneDataDir;
	fileName += sceneName;
	fileName += kSceneDataSuffix;

	// Read the current scene data to read the Version
	uint32 version = 0;

	try
	{
		FString readSceneString = fileManager.ReadFileToString(fileName);
		json::JSON readSceneJson = json::JSON::Load(readSceneString);

		if (!readSceneJson.hasKey("Version") || readSceneJson.at("Version").JSONType() != json::JSON::Class::Integral)
		{
			version = 0;
		}
		else
		{
			version = readSceneJson.at("Version").ToInt();
		}
	}
	catch (const std::exception& e)
	{
		// If the file does not exist or cannot be read, we can assume it's a new scene and set version to 0
		version = 0;
	}

	json::JSON writeSceneJson = json::JSON::Make(json::JSON::Class::Object);
	json::JSON worldJson = json::JSON::Make(json::JSON::Class::Object);
	mCurrentWorld->SerializeClass(worldJson);

	writeSceneJson["Version"] = version;
	writeSceneJson["NextUUID"] = UEngineStatics::GetNextUUID();
	writeSceneJson["World"] = worldJson;

	FString jsonString = FString(writeSceneJson.dump(1, "  "));
	fileManager.WriteStringToFile(fileName, jsonString);
}

void FSceneManager::LoadScene(std::string_view filePath, const FFileManager& fileManager)
{
	FString jsonString;

	try
	{
		jsonString = fileManager.ReadFileToString(filePath);
	}
	catch (const std::exception& e)
	{
		UE_LOG_F(Error, Core, "Failed to read scene file {}: {}", filePath, e.what());
		return;
	}

	try
	{
		json::JSON readSceneJson = json::JSON::Load(jsonString);

		if (!readSceneJson.hasKey("NextUUID") || readSceneJson.at("NextUUID").JSONType() != json::JSON::Class::Integral)
		{
			throw std::runtime_error("Scene file does not contain a valid NextUUID.");
		}

		if (!readSceneJson.hasKey("World") || readSceneJson.at("World").JSONType() != json::JSON::Class::Object)
		{
			throw std::runtime_error("Scene file does not contain a valid World.");
		}

		const uint32 nextUUID = readSceneJson.at("NextUUID").ToInt();
		const json::JSON& worldJson = readSceneJson.at("World");

		UWorld* newWorld = FObjectFactory::LoadObject<UWorld>(worldJson);

		if (!newWorld)
		{
			throw std::runtime_error("Failed to load world.");
		}

		delete mCurrentWorld;
		mCurrentWorld = newWorld;

		UEngineStatics::SetNextUUID(nextUUID);
		ResetSelectedActor();
	}
	catch (const std::exception& e)
	{
		UE_LOG_F(Error, Core, "Failed to load scene file {}: {}", filePath, e.what());
	}
}

void FSceneManager::RemoveActor(AActor* actor)
{
	if (mSelectedActor == actor)
	{
		ResetSelectedActor();
	}

	assert(mCurrentWorld != nullptr);
	mCurrentWorld->RemoveActor(actor->UUID);

	// TODO?: Consider whether to delete the actor here or manage its lifetime elsewhere.
	delete actor;
}

void  FSceneManager::SetSelectedActor(AActor* actor)
{
	if (actor == nullptr)
	{
		UE_LOG_F(Warning, Core, "SetSelectedActor: Attempted to set selected actor to nullptr.");
		return;
	}

	if (actor == mSelectedActor)
	{
		UE_LOG_F(Log, Core, "SetSelectedActor: Actor with UUID {} is already selected.", actor->UUID);
		return; // No change
	}

	UE_LOG_F(Log, Core, "SetSelectedActor: Actor with UUID {} is now selected.", actor->UUID);
	mSelectedActor = actor;
}

float FSceneManager::GetPanelWidth() const
{
	return mPanelWidth;
}

const TArray<FRenderInfo>& FSceneManager::GetRenderInfos() const
{
	if (mCurrentWorld)
	{
		return mCurrentWorld->GetRenderInfos();
	}

	return TArray<FRenderInfo>();
}

const TArray<FRenderInfo>& FSceneManager::GetAxisRenderInfos() const
{
	static TArray<FRenderInfo> axisRenderInfos;

	if (axisRenderInfos.IsEmpty())
	{
		FRenderInfo renderInfo{};
		renderInfo.eRenderFlags = ERenderFlags::RF_WorldAxis;

		axisRenderInfos.Add(renderInfo);
	}

	return axisRenderInfos;
}

//FString FSceneManager::mOpenSceneFileDialog() const
//{
//	char fileName[MAX_PATH] = {};
//	OPENFILENAMEA openFileName = {};
//
//	openFileName.lStructSize = sizeof(OPENFILENAMEA);
//	openFileName.hwndOwner = static_cast<HWND>(ImGui::GetMainViewport()->PlatformHandleRaw);  // main window
//
//	openFileName.lpstrFilter = "Scene Files (*.Scene)\0*.Scene\0All Files (*.*)\0*.*\0";
//	openFileName.lpstrFile = fileName;
//	openFileName.nMaxFile = MAX_PATH;
//
//	openFileName.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_NOCHANGEDIR;
//	openFileName.lpstrDefExt = "Scene";
//
//	std::filesystem::path initialDirectory = std::filesystem::absolute(std::filesystem::path("Assets") / "SceneData");
//
//	if (!std::filesystem::exists(initialDirectory))
//	{
//		std::filesystem::create_directories(initialDirectory);
//	}
//
//	const std::string initialDirectoryString = initialDirectory.string();
//
//	openFileName.lpstrInitialDir = initialDirectoryString.c_str();
//
//	if (GetOpenFileNameA(&openFileName))
//	{
//		return FString(fileName);
//	}
//
//	return FString("");
//}

//FString FSceneManager::mSaveSceneFileDialog() const
//{
//	char fileName[MAX_PATH] = {};
//	OPENFILENAMEA openFileName = {};
//
//	openFileName.lStructSize = sizeof(OPENFILENAMEA);
//	openFileName.hwndOwner = static_cast<HWND>(ImGui::GetMainViewport()->PlatformHandleRaw);  // main window
//
//	openFileName.lpstrFilter = "Scene Files (*.Scene)\0*.Scene\0All Files (*.*)\0*.*\0";
//	openFileName.lpstrFile = fileName;
//	openFileName.nMaxFile = MAX_PATH;
//
//	openFileName.Flags = OFN_EXPLORER | OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY | OFN_NOCHANGEDIR;
//	openFileName.lpstrDefExt = "Scene";
//
//	std::filesystem::path initialDirectory = std::filesystem::absolute(std::filesystem::path("Assets") / "SceneData");
//
//	if (!std::filesystem::exists(initialDirectory))
//	{
//		std::filesystem::create_directories(initialDirectory);
//	}
//
//	const std::string initialDirectoryString = initialDirectory.string();
//
//	openFileName.lpstrInitialDir = initialDirectoryString.c_str();
//
//	if (GetSaveFileNameA(&openFileName))
//	{
//		return FString(fileName);
//	}
//
//	return FString("");
//}

//
//FSceneData FSceneManager::ReadSceneData(
//	std::string_view sceneName,
//	const FFileManager& fileManager)
//{
//	FString fileName = sceneName;
//	fileName += kSceneDataSuffix;
//
//	json::JSON jsonData = json::JSON::Load(fileManager.ReadFileToString(fileName));
//	FSceneData sceneData = FSceneData(jsonData);
//	return sceneData;
//}
//
//UWorld* FSceneManager::BuildWorldFromSceneData(const FSceneData& sceneData)
//{
//	//UWorld* newWorld = FObjectFactory::ConstructObject<UWorld>();
//
//	//for (const auto& [UUID, primitiveData] : sceneData.Primitives) 
//	//{
//	//	// TODO: Replace AActor creation logic later
//	//	AActor* newActor = FObjectFactory::ConstructObject<AActor>();
//	//	UPrimitiveComponent* newPrimitiveComponent =
//	//		FObjectFactory::ConstructObject<UPrimitiveComponent>(
//	//			);
//	//}
//
//	//UEngineStatics::SetNextUUID(sceneData.NextUUID);
//	throw std::logic_error("BuildWorldFromSceneData is not implemented yet.");
//	return nullptr;
//}
