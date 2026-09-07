
#include "Object.h"
#include "EngineStatics.h"
#include "Json/json.hpp"

TSparseArray<UObject*> UObject::GUObjectArray;

UObject* FClassInfo::CreateInstance() const
{
	if (Constructor)
	{
		return Constructor();
	}
	return nullptr;
}


UObject::UObject()
{
	InternalIndex = GUObjectArray.Add(this);
}

UObject::~UObject()
{
	/*
	// Ensure that the object is in the GUObjectArray before attempting to remove it
	if (GUObjectArray.Num() < InternalIndex || GUObjectArray[InternalIndex] != this)
	{
		assert(false && "Invalid InternalIndex or GUObjectArray mismatch.");
		return;
	}
	*/

	GUObjectArray.RemoveAt(InternalIndex);
}

void UObject::Initialize()
{
	UUID = UEngineStatics::GenerateUUID();
}

const FClassInfo* UObject::GetClass()
{
	static FClassInfo classInstance = FClassInfo(
		"UObject",
		nullptr,
		[]() -> UObject* { return new UObject(); }
	);
	return &classInstance;
}

void UObject::SerializeClass(json::JSON& outJson) const
{
	outJson["ClassName"] = GetRuntimeClass()->Name;

	json::JSON propertiesJson = json::JSON::Make(json::JSON::Class::Object);
	propertiesJson["UUID"] = UUID;
	outJson["Properties"] = propertiesJson;
}

void UObject::DeserializeClass(const json::JSON& inJson)
{
	if (!inJson.hasKey("Properties") || inJson.at("Properties").JSONType() != json::JSON::Class::Object)
	{
		throw std::runtime_error("Invalid JSON format for Properties");
	}
	const json::JSON& propertiesJson = inJson.at("Properties");

	if (!propertiesJson.hasKey("UUID") || propertiesJson.at("UUID").JSONType() != json::JSON::Class::Integral)
	{
		throw std::runtime_error("Invalid JSON format for UUID");
	}

	UUID = propertiesJson.at("UUID").ToInt();
}

bool UObject::IsA(const FClassInfo* classInfo) const
{
	const FClassInfo* currentClass = GetRuntimeClass();
	while (currentClass)
	{
		if (currentClass == classInfo)
		{
			return true;
		}
		currentClass = currentClass->SuperClass;
	}
	return false;
}
