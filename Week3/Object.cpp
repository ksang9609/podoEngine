
#include "Object.h"
#include "EngineStatics.h"
#include "Json/json.hpp"
#include "Name.h"

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
	GUObjectRevision++;
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
	GUObjectRevision++;
}

void UObject::Destroy()
{
	delete this;
}

void UObject::Initialize()
{
	UUID = UEngineStatics::GenerateUUID();
}

FClassInfo UObject::ClassInfo(
	"UObject",
	nullptr,
	[]() -> UObject*
	{
		return new UObject();
	}
);

const FClassInfo* UObject::GetClass()
{
	return &ClassInfo;
}

void UObject::SerializeClass(json::JSON& outJson) const
{
	outJson["ClassName"] = GetRuntimeClass()->Name;

	json::JSON propertiesJson = json::JSON::Make(json::JSON::Class::Object);
	propertiesJson["UUID"] = UUID;
	propertiesJson["Name"] = mName.ToString();
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
	if (propertiesJson.hasKey("Name") && propertiesJson.at("Name").JSONType() == json::JSON::Class::String)
	{
		const FString loadedName(propertiesJson.at("Name").ToString());

		mName = FName(loadedName);
	}
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

UObject* UObject::GetObjectByUUID(int32 uuid)
{
	for (const auto& object : GUObjectArray)
	{
		if (object && object->UUID == uuid)
		{
			return object;
		}
	}
	return nullptr;
}

UObject* UObject::GetObjectByInternalIndex(uint32 internalIndex)
{
	if (GUObjectArray.IsValidIndex(internalIndex))
	{
		return GUObjectArray[internalIndex];
	}
	return nullptr;
}
