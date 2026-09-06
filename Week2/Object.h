#pragma once

#include <functional>

#include "Core.h"
#include "TArray.h"


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

class UObject
{
public:
	// Todo: Fix
	uint32 UUID;
	uint32 InternalIndex;

	virtual ~UObject();

	void Initialize() {};

	// StaticClass() in Unreal Engine
	static const FClassInfo* GetClass();

	// GetClass() in Unreal Engine
	inline const FClassInfo* GetRuntimeClass() const { return mClassInfo; }

	template<typename TObject>
	bool IsA() const;

	bool IsA(const FClassInfo* classInfo) const;

protected:
	UObject();

private:
	static TArray<UObject*> GUObjectArray;

	friend struct FObjectFactory;
	const FClassInfo* mClassInfo;
};


#include  "Object.inl"
