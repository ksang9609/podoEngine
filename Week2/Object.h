#pragma once

#include <functional>

#include "Core.h"
#include "TArray.h"


class UObject;

//using ConstructorFunc = UObject * (*)();

struct FClassInfo
{
	FString Name;
	FClassInfo* SuperClass;
	std::function<UObject* ()> Constructor;

	FClassInfo(FString name, FClassInfo* superClass, std::function<UObject* ()> constructor)
		: Name(std::move(name)), SuperClass(superClass), Constructor(constructor) {
	}

	UObject* CreateInstance() const;

private:
};

struct FObjectFactory
{
	static UObject* ConstructObject(FClassInfo* classInfo);

	template<typename TObject, typename... Args>
		requires std::derived_from<TObject, UObject>
	static TObject* ConstructObject(Args&& ...args);
};

class UObject
{
public:
	// Todo: Fix
	uint32 UUID;
	uint32 InternalIndex;

	UObject();
	virtual ~UObject();

	void Initialize() {};

	// StaticClass() in Unreal Engine
	static FClassInfo* GetClass();

	// GetClass() in Unreal Engine
	inline virtual FClassInfo* GetRuntimeClass() const { return GetClass(); }

	template<typename TObject>
	bool IsA() const;

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
			[]() -> UObject* {														\
				UObject* instance = new className();								\
				instance->Initialize();												\
				return instance;													\
			}																		\
		);																			\
		return &classInstance;														\
	}																				\
	virtual FClassInfo* GetRuntimeClass() const override { return GetClass(); }		\
private:

#include  "Object.inl"
