#include "ObjectFactory.h"

#include "ThirdParty/Json/json.hpp"

#include "Engine/Actor.h"
#include "Engine/Components/PrimitiveComponent.h"
#include "Engine/Components/NameComponent.h"
#include "Rendering/FontResource.h"

#include "Object.h"

const FFontResource* FObjectFactory::mDefaultFontResource = nullptr;

void FObjectFactory::Initialize(const FFontResource& fontResource)
{
	mDefaultFontResource = &fontResource;
}

const FFontResource* FObjectFactory::GetDefaultFontResource()
{
	return mDefaultFontResource;
}

UObject* FObjectFactory::ConstructUnInitializedObject(const FClassInfo* classInfo)
{
	if (!classInfo || !classInfo->Constructor)
	{
		return nullptr;
	}

	UObject* instance = classInfo->CreateInstance();

	if (instance)
	{
		instance->mClassInfo = classInfo;
		instance->mName = FName(classInfo->Name);
	}
	return instance;
}

UObject* FObjectFactory::LoadObject(const FClassInfo* classInfo, const json::JSON& inJson)
{
	UObject* instance = ConstructUnInitializedObject(classInfo);

	if (instance)
	{
		instance->DeserializeClass(inJson);
	}
	return instance;
}

AActor* FObjectFactory::SpawnPrimitiveActor(
	EPrimitive primitiveType,
	FVector3 Location, FRotator Rotation, FVector3 Scale)
{
	FName PrimitiveName(PrimitiveToString(primitiveType));

	AActor* actor = ConstructObjectWithName<AActor>(PrimitiveName);

	UPrimitiveComponent* component = ConstructObject<UPrimitiveComponent>(
		primitiveType, Location, Rotation, Scale);

	actor->AddRootSceneComponent(component);

	/* DEBUG */
	assert(mDefaultFontResource && "FObjectFactory::Initialize must be called before SpawnPrimitiveActor.");
	UNameComponent& billboardComponent = actor->CreateAndAddComponent<UNameComponent>(
		actor->GetName().ToString(), FVector3{0, 0, 1}, *mDefaultFontResource);
	billboardComponent.AttachTo(*component);
	return actor;
}

const FClassInfo* FObjectFactory::GetClassInfoByName(const FString& className)
{
	if (!mClassInfoMap.Contains(className))
	{
		return nullptr;
	}

	return mClassInfoMap[className]();
}

bool FObjectFactory::RegisterClassInfo(FString className, const FClassInfo* classInfo)
{
	if (mClassInfoMap.Contains(className))
	{
		return false;
	}
	mClassInfoMap.Add(className, [classInfo]() -> const FClassInfo* { return classInfo; });
	return true;
}

#include "Engine/Components/SceneComponent.h"
#include "Engine/Components/CubeComponent.h"
#include "Engine/Components/SphereComponent.h"
#include "Engine/World.h"
#include "Engine/Components/BillboardComponent.h"

TMap<FString, std::function<const FClassInfo* ()>> FObjectFactory::mClassInfoMap = {
	{"UObject", &UObject::GetClass },
	{"AActor", &AActor::GetClass },
	{"UActorComponent", &UActorComponent::GetClass },
	{"USceneComponent", &USceneComponent::GetClass },
	{"UPrimitiveComponent", &UPrimitiveComponent::GetClass },
	{"UCubeComponent", &UCubeComponent::GetClass },
	{"USphereComponent", &USphereComponent::GetClass },
	{"UBillboardComponent", &UBillboardComponent::GetClass },
	{"UWorld", &UWorld::GetClass },
	{"UNameComponent",& UNameComponent::GetClass }
};
