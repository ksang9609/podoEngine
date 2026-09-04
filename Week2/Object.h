#pragma once

// TODO: Replace std::string with FString later
#include <string>

#include "Core.h"

class UObject;

using ConstructorFunc = UObject * (*)();

struct FClassInfo
{
	std::string Name;
	FClassInfo* SuperClass;
	ConstructorFunc Constructor;

	FClassInfo(std::string name, FClassInfo* superClass, ConstructorFunc constructor)
		: Name(std::move(name)), SuperClass(superClass), Constructor(constructor) {
	}

	UObject* CreateInstance() const;

private:
};

class UObject
{
public:
	uint32 UUID;
	uint32 InternalIndex;

	UObject();
	virtual ~UObject() = default;

	// StaticClass() in Unreal Engine
	static FClassInfo* GetClass();

	// GetClass() in Unreal Engine
	inline virtual FClassInfo* GetRuntimeClass() const { return GetClass(); }

	template<typename TObject>
	bool IsA() const
	{
		return IsA(TObject::GetClass());
	}

	bool IsA(FClassInfo* classInfo) const;

protected:
};

#define REFLECT_CLASS(className, superClassName)									\
public:																				\
	static FClassInfo* GetClass()													\
	{																				\
		static FClassInfo classInstance = FClassInfo(								\
			#className,																\
			superClassName::GetClass(),												\
			[]() -> UObject* { return new className(); }							\
		);																			\
		return &classInstance;														\
	}																				\
	virtual FClassInfo* GetRuntimeClass() const override { return GetClass(); }		\
private:
