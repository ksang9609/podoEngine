#include "ObjectFactory.h"

#include "Json/json.hpp"

#include "Actor.h"

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
	// Create a new actor
	AActor* actor = ConstructObject<AActor>();

	/**
	* TODO: Add a component based on the primitiveType.
	*/

	return actor;
}

const FClassInfo* FObjectFactory::GetClassInfoByName(const FString& className)
{
	return mClassInfoMap[className]();
}

#include "SceneComponent.h"
#include "PrimitiveComponent.h"
#include "CubeComponent.h"
#include "SphereComponent.h"
#include "World.h"

const TMap<FString, std::function<const FClassInfo* ()>> FObjectFactory::mClassInfoMap = {
	{"UObject", &UObject::GetClass },
	{"AActor", &AActor::GetClass },
	{"UActorComponent", &UActorComponent::GetClass },
	{"USceneComponent", &USceneComponent::GetClass },
	{"UPrimitiveComponent", &UPrimitiveComponent::GetClass },
	{"UCubeComponent", &UCubeComponent::GetClass },
	{"USphereComponent", &USphereComponent::GetClass },
	{"UWorld", &UWorld::GetClass }
};
