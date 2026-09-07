#pragma once

#include <functional>

#include "Core.h"
#include "TArray.h"
#include "TSparseArray.h"
#include "ObjectFactory.h"


namespace json { class JSON; }

class UObject;

//using ConstructorFunc = UObject * (*)();

struct FClassInfo
{
	FString Name;
	const FClassInfo* SuperClass;
	std::function<UObject* ()> Constructor;

	FClassInfo(FString name, const FClassInfo* superClass, std::function<UObject* ()> constructor)
		: Name(std::move(name)), SuperClass(superClass), Constructor(constructor) {
	}

	UObject* CreateInstance() const;

private:
};

struct FObjectID
{
	int32 UUID;
	int32 InternalIndex;
};

class UObject
{
public:
	// Todo: Fix
	int32 UUID;
	FObjectID ObjectID;
	uint32 InternalIndex;

	virtual ~UObject();

	void Initialize();

	// StaticClass() in Unreal Engine
	static const FClassInfo* GetClass();

	// GetClass() in Unreal Engine
	inline const FClassInfo* GetRuntimeClass() const { return mClassInfo; }

	// TODO?: Replace json type with a more generic type, such as a variant or a map
	virtual void SerializeClass(json::JSON& outJson) const;
	virtual void DeserializeClass(const json::JSON& inJson);

	template<typename TObject>
	bool IsA() const;

	bool IsA(const FClassInfo* classInfo) const;

protected:
	UObject();

private:
	static TSparseArray<UObject*> GUObjectArray;

	friend struct FObjectFactory;
	const FClassInfo* mClassInfo;
};


#include  "Object.inl"
