#pragma once

#include <functional>

#include "enum.h"
#include "Vector.h"
#include "Rotator.h"

class UObject;
class AActor;
class FClassInfo;

struct FObjectFactory
{
	static UObject* ConstructObject(const FClassInfo* classInfo);

	template<typename TObject, typename... Args>
		requires std::derived_from<TObject, UObject>
	static TObject* ConstructObject(Args&& ...args);

	static AActor* SpawnPrimitiveActor(EPrimitive primitiveType,
		FVector3 Location, FRotator Rotation, FVector3 Scale
	);
};

template<typename TObject, typename... Args>
	requires std::derived_from<TObject, UObject>
TObject* FObjectFactory::ConstructObject(Args&& ...args)
{
	static_assert(requires(TObject * obj)
	{
		obj->Initialize(std::forward<Args>(args)...);

	}, "TObject must have an Initialize method that accepts the provided arguments.");

	const FClassInfo* classInfo = TObject::GetClass();
	if (!classInfo || !classInfo->Constructor)
	{
		return nullptr;
	}

	TObject* instance = static_cast<TObject*>(classInfo->CreateInstance());
	if (instance)
	{
		instance->mClassInfo = classInfo;
		instance->Initialize(std::forward<Args>(args)...);
	}
	return instance;
}
