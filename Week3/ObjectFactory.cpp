#include "ObjectFactory.h"

#include "Json/json.hpp"

#include "Actor.h"
#include "PrimitiveComponent.h"
#include "Object.h"

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
	FName PrimitiveName;

	switch (primitiveType)
	{
	case EPrimitive::EP_Cube:
		PrimitiveName = FName("Cube");
		break;

	case EPrimitive::EP_Sphere:
		PrimitiveName = FName("Sphere");
		break;

	case EPrimitive::EP_Triangle:
		PrimitiveName = FName("Triangle");
		break;

	case EPrimitive::EP_GizmoArrow:
		PrimitiveName = FName("GizmoArrow");
		break;

	case EPrimitive::EP_Circle:
		PrimitiveName = FName("Circle");
		break;

	default:
		PrimitiveName = FName("Unknown");
		break;
	}

	AActor* actor = ConstructObjectWithName<AActor>(PrimitiveName);

	UPrimitiveComponent* component = ConstructObject<UPrimitiveComponent>(
		primitiveType, Location, Rotation, Scale);

	actor->AddRootSceneComponent(component);

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

#include "SceneComponent.h"
#include "CubeComponent.h"
#include "SphereComponent.h"
#include "World.h"

TMap<FString, std::function<const FClassInfo* ()>> FObjectFactory::mClassInfoMap = {
	{"UObject", &UObject::GetClass },
	{"AActor", &AActor::GetClass },
	{"UActorComponent", &UActorComponent::GetClass },
	{"USceneComponent", &USceneComponent::GetClass },
	{"UPrimitiveComponent", &UPrimitiveComponent::GetClass },
	{"UCubeComponent", &UCubeComponent::GetClass },
	{"USphereComponent", &USphereComponent::GetClass },
	{"UWorld", &UWorld::GetClass }
};
