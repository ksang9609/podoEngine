#include "Actor.h"

#include <format>

#include "JsonUtil.h"

AActor::~AActor()
{
	for (UActorComponent* removeComponent : mComponents)
	{
		delete removeComponent;
	}
}

void AActor::Initialize()
{
	// Todo: Change to false 
	mbClicked = true;
}

void AActor::SerializeClass(json::JSON& outJson) const
{
	UObject::SerializeClass(outJson);
	json::JSON componentsJson = json::JSON::Make(json::JSON::Class::Array);

	for (const UActorComponent* component : mComponents)
	{
		json::JSON componentJson;
		component->SerializeClass(componentJson);
		componentsJson.append(std::move(componentJson));
	}
	outJson["Properties"]["mComponents"] = componentsJson;
}

void AActor::DeserializeClass(const json::JSON& inJson)
{
	UObject::DeserializeClass(inJson);

	const json::JSON& propertiesJson = inJson.at("Properties");

	if (!propertiesJson.hasKey("mComponents") || propertiesJson.at("mComponents").JSONType() != json::JSON::Class::Array)
	{
		throw std::runtime_error(std::format("{}: mComponents requires an array", GetRuntimeClass()->Name));
	}

	const json::JSON& componentsJson = propertiesJson.at("mComponents");

	for (const auto& componentJson : componentsJson.ArrayRange())
	{
		if (!componentJson.hasKey("ClassName") || componentJson.at("ClassName").JSONType() != json::JSON::Class::String)
		{
			throw std::runtime_error(std::format("{}: ClassName requires a string", GetRuntimeClass()->Name));
		}
		FString className(componentJson.at("ClassName").ToString());

		const FClassInfo* classInfo = FObjectFactory::GetClassInfoByName(className);
		if (!classInfo)
		{
			throw std::runtime_error(std::format("{}: Unknown class name: {}", GetRuntimeClass()->Name, className));
		}
		UActorComponent* component = static_cast<UActorComponent*>(FObjectFactory::LoadObject(classInfo, componentJson));
		AddComponent(component);
	}
}

void AActor::AddComponent(UActorComponent* actorComponent)
{
	assert(actorComponent);
	assert(getComponentIndex(actorComponent->UUID) == -1);

	mComponents.Add(actorComponent);
}

bool AActor::RemoveComponent(uint32 componentUUID)
{
	int32 componentIndex = getComponentIndex(componentUUID);
	if (componentIndex == -1)
	{
		return false;
	}

	mComponents.RemoveAt(componentIndex, 1);

	return true;
}


void AActor::Update(TArray<FRenderInfo>* outRenderInfos)
{
	for (UActorComponent* component : mComponents)
	{
		component->Update(outRenderInfos);
	}
}

void AActor::Click()
{
	mbClicked = true;
}

void AActor::UnClick()
{
	mbClicked = false;
}

bool AActor::IsClicked()
{
	return mbClicked;
}

int32 AActor::getComponentIndex(uint32 componentUUID) const
{
	for (uint32 i = 0; i < mComponents.Num(); ++i)
	{
		if (mComponents[i]->UUID == componentUUID)
		{
			return i;
		}
	}

	return -1;
}
