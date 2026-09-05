
#include "Object.h"
#include "EngineStatics.h"

TArray<UObject*> UObject::GUObjectArray;

UObject* FClassInfo::CreateInstance() const
{
	if (Constructor)
	{
		return Constructor();
	}
	return nullptr;
}

UObject* FObjectFactory::ConstructObject(FClassInfo* classInfo)
{
	if (!classInfo || !classInfo->Constructor)
	{
		return nullptr;
	}
	return classInfo->CreateInstance();
}

UObject::UObject()
{
	UUID = UEngineStatics::GenerateUUID();
	InternalIndex = GUObjectArray.Add(this);
}

UObject::~UObject()
{
	GUObjectArray.RemoveAt(InternalIndex, 1);

	for (uint32 index = InternalIndex; index < GUObjectArray.Num(); ++index)
	{
		--(GUObjectArray[index]->InternalIndex);
	}
}

FClassInfo* UObject::GetClass()
{
	static FClassInfo classInstance = FClassInfo(
		"UObject",
		nullptr,
		[]() -> UObject* { return new UObject(); }
	);
	return &classInstance;
}

bool UObject::IsA(FClassInfo* classInfo) const
{
	FClassInfo* currentClass = GetRuntimeClass();
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
