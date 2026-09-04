#pragma once

#include "Core.h"
#include "TArray.h"

class UObject;

using ConstructorFunc = UObject * (*)();

struct FClassInfo
{
	FString Name;
	FClassInfo* SuperClass;
	ConstructorFunc Constructor;

	FClassInfo(FString name, FClassInfo* superClass, ConstructorFunc constructor)
		: Name(std::move(name)), SuperClass(superClass), Constructor(constructor) {
	}

	UObject* CreateInstance() const;

private:
};

struct FObjectFactory
{
	static UObject* ConstructObject(FClassInfo* classInfo);

	template<typename TObject> requires std::derived_from<TObject, UObject>
	static TObject* ConstructObject()
	{
		return static_cast<TObject*>(ConstructObject(TObject::GetClass()));
	}
};

class UObject
{
public:
	// Todo: Fix
	uint32 UUID;
	uint32 InternalIndex;
	
	UObject();
	virtual ~UObject();

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

private:
	static TArray<UObject*> GUObjectArray;

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
