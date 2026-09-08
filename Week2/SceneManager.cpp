
#include "SceneManager.h"

#include <format>

#include "FileManager.h"
#include "EngineStatics.h"
#include "FileManager.h"
#include "JsonUtil.h"
#include "ObjectFactory.h"
#include "PrimitiveComponent.h"
#include "TArray.h"
#include "World.h"
#include "FEditorViewportClient.h"
#include "Camera.h"
#include "Console.h"

#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_dx11.h"
#include "imGui/imgui_impl_win32.h"

#include "FrameTimer.h"
#include "CubeComponent.h"

FSceneManager::FSceneManager()
{
	//mCurrentWorld = FObjectFactory::ConstructObject<UWorld>();

	// Todo: Test code, move to other function
	//{
	//	UCubeComponent* cubeComponent = FObjectFactory::ConstructObject<UCubeComponent>(FVector(0), FRotator(), FVector(1));
	//	AActor* cubeActor = FObjectFactory::ConstructObject<AActor>();
	//	cubeActor->AddComponent(cubeComponent);
	//	mCurrentWorld->AddActor(cubeActor);

	//	UCubeComponent* cubeComponent2 = FObjectFactory::ConstructObject<UCubeComponent>(FVector(1, 1, 1), FRotator(), FVector(0.5));
	//	AActor* cubeActor2 = FObjectFactory::ConstructObject<AActor>();
	//	cubeActor2->AddComponent(cubeComponent2);
	//	mCurrentWorld->AddActor(cubeActor2);
	//}
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

void UpdateControlPanelGUI(const FGuiReference& guiReference);
void UpdatePropertyWindowGUI(const FGuiReference& guiReference);

void FSceneManager::UpdateGUI(const FGuiReference& guiReference)
{
	//ImGui
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	
	UpdateControlPanelGUI(guiReference);
	UpdatePropertyWindowGUI(guiReference);

	ConsoleWindow::GetInstance().Draw();
}

void UpdateControlPanelGUI(const FGuiReference& guiReference)
{
	ImGui::Begin("Jungle Control Panel");
	ImGui::Text("Hello Jungle World!");
	ImGui::Text("FPS: %.1f  dt: %.4f", guiReference.FrameTimer.GetFPS(), guiReference.FrameTimer.GetDeltaTime());

	ImGui::Separator();
	//ImGui::SliderFloat("Speed", &Camera.Speed, -10.0f, 10.0f);
	if (ImGui::BeginCombo("##ShowFlags", "Show Flags"))
	{
		bool bWireFrame = guiReference.GraphicsManager->GetWireFrame();
		ImGui::Checkbox("Wire frame", &bWireFrame);
		guiReference.GraphicsManager->SetWireFrame(bWireFrame);
		ImGui::EndCombo();
	}
	ImGui::Text("FOV     ");
	ImGui::SameLine();
	//ImGui::SliderFloat("##FOV", &fovDegree, 0.0f, 180.0f);

	// 1) 라벨 텍스트를 먼저 그리고 같은 줄로
	ImGui::Text("Location");
	ImGui::SameLine();

	// 2) 텍스트를 그린 "뒤"의 남은 폭을 기준으로 계산
	const float spacing = ImGui::GetStyle().ItemSpacing.x;
	const float itemWidth = (ImGui::GetContentRegionAvail().x - spacing * 2.0f) / 3.0f;


	FCamera& camera = guiReference.ViewportClient->GetCamera();
	URenderer* renderer = guiReference.GraphicsManager->GetRenderer();

	ImGui::SetNextItemWidth(itemWidth);
	ImGui::DragFloat("##CamLocX", &camera.Transform.Location.x, -10.0f, 10.0f);
	ImGui::SameLine();
	ImGui::SetNextItemWidth(itemWidth);
	ImGui::DragFloat("##CamLocY", &camera.Transform.Location.y, -10.0f, 10.0f);
	ImGui::SameLine();
	ImGui::SetNextItemWidth(itemWidth);
	ImGui::DragFloat("##CamLocZ", &camera.Transform.Location.z, -10.0f, 10.0f);

	ImGui::Text("Rotation");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(itemWidth);
	ImGui::DragFloat("##CamRotX", &camera.Transform.Rotation.Roll, -10.0f, 180.0f);
	ImGui::SameLine();
	ImGui::SetNextItemWidth(itemWidth);
	ImGui::DragFloat("##CamRotY", &camera.Transform.Rotation.Pitch, -10.0f, 180.0f);
	ImGui::SameLine();
	ImGui::SetNextItemWidth(itemWidth);
	ImGui::DragFloat("##CamRotZ", &camera.Transform.Rotation.Yaw, -10.0f, 180.0f);
	//ImGui::Checkbox("Depth Test", &renderer->bDepthTestEnabled);
	//ImGui::TextUnformatted(renderer->bDepthTestEnabled
	//	? "ON : orange (near) stays in front"
	//	: "OFF: blue (far, drawn last) overwrites");

	ImGui::End();
}

void UpdatePropertyWindowGUI(const FGuiReference& guiReference)
{
	ImGui::Begin("Jungle Property Window");
	if (guiReference.ViewportClient->ClickedActor)
	{
		// Temporary variables to hold the values for ImGui input fields
		const FTransform& originalTransform = guiReference.ViewportClient->ClickedActor->GetTransform();

		// Get the current transform of the clicked actor
		FVector translationInput = originalTransform.Location;
		FRotator rotationInput = originalTransform.Rotation;
		FVector scaleInput = originalTransform.Scale;

		// Display and edit the transform properties using ImGui input fields
		if (ImGui::InputFloat3("Translation", &translationInput.x))
		{
			guiReference.ViewportClient->ClickedActor->SetLocation(translationInput);
		}
		if (ImGui::InputFloat3("Rotation", &rotationInput.Pitch))
		{
			guiReference.ViewportClient->ClickedActor->SetRotation(rotationInput);
		}
		if (ImGui::InputFloat3("Scale", &scaleInput.x))
		{
			guiReference.ViewportClient->ClickedActor->SetScale(scaleInput);
		}
	}
	ImGui::End();
}

void FSceneManager::NewScene()
{
	if (mCurrentWorld != nullptr)
	{
		delete mCurrentWorld;
	}

	//mCurrentWorld = FObjectFactory::ConstructObject<UWorld>();


}

void FSceneManager::DeleteScene()
{
	//delete mCurrentWorld;
}

void FSceneManager::SaveScene(
	std::string_view sceneName,
	const FFileManager& fileManager)
{
	FString fileName = kSceneDataDir;
	fileName += FString("/");
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

void FSceneManager::LoadScene(
	std::string_view sceneName,
	const FFileManager& fileManager)
{
	FString fileName = kSceneDataDir;
	fileName += FString("/");
	fileName += sceneName;
	fileName += kSceneDataSuffix;

	FString jsonString = fileManager.ReadFileToString(fileName);

	json::JSON readSceneJson = json::JSON::Load(jsonString);

	if (!readSceneJson.hasKey("NextUUID") || readSceneJson.at("NextUUID").JSONType() != json::JSON::Class::Integral)
	{
		throw std::runtime_error(std::format("Scene file {} does not contain a valid NextUUID field.", fileName));
	}
	uint32 nextUUID = readSceneJson.at("NextUUID").ToInt();
	json::JSON worldJson = readSceneJson.at("World");

	UWorld* newWorld = FObjectFactory::LoadObject<UWorld>(worldJson);
	if (!newWorld)
	{
		throw std::runtime_error(std::format("Failed to load world from scene: {}", sceneName));
	}
	UEngineStatics::SetNextUUID(nextUUID);

	// Replace the contents of mCurrentWorld with newWorld
	delete mCurrentWorld;
	mCurrentWorld = newWorld;
}

const TArray<FRenderInfo> FSceneManager::GetRenderInfos()
{
	if (mCurrentWorld)
	{
		return mCurrentWorld->GetRenderInfos();
	}

	return TArray<FRenderInfo>();
}

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
