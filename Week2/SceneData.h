#pragma once

//#include "Json/json.hpp"

#include "Core.h"
#include "TArray.h"
#include "TMap.h"
#include "Vector.h"
#include "Rotator.h"
#include "enum.h"

namespace json
{
	class JSON;
}

struct FPrimitiveData
{
	FVector Location;
	FRotator Rotation;
	FVector Scale;
	EPrimitive PrimitiveType;

	FPrimitiveData();
	FPrimitiveData(json::JSON);

	json::JSON ToJson() const;
	FString ToJsonString() const;
};

struct FSceneData
{
	uint32 Version;
	uint32 NextUUID;
	TMap<uint32, FPrimitiveData> Primitives;

	FSceneData();
	FSceneData(json::JSON);

	json::JSON ToJson() const;
	FString ToJsonString() const;
};

json::JSON FVectorToJson(const FVector& Vector);
json::JSON FRotatorToJson(const FRotator& Rotator);
json::JSON EPrimitiveToJson(const EPrimitive& Primitive);

FVector FVectorFromJson(const json::JSON& json);
FRotator FRotatorFromJson(const json::JSON& json);
EPrimitive EPrimitiveFromJson(const json::JSON& json);

