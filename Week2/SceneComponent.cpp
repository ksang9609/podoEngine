#include "SceneComponent.h"

#include <format>

#include "Transform.h"
#include "JsonUtil.h"

USceneComponent::USceneComponent()
{
}

void USceneComponent::Initialize(FVector location, FRotator rotation, FVector scale3D)
{
	mRelativeLocation = location;
	mRelativeRotation = rotation;
	mRelativeScale3D = scale3D;
}

USceneComponent::~USceneComponent()
{
}

void USceneComponent::SerializeClass(json::JSON& outJson) const
{
	UActorComponent::SerializeClass(outJson);
	outJson["Properties"]["RelativeLocation"] = FVectorToJson(mRelativeLocation);
	outJson["Properties"]["RelativeRotation"] = FRotatorToJson(mRelativeRotation);
	outJson["Properties"]["RelativeScale3D"] = FVectorToJson(mRelativeScale3D);
}

void USceneComponent::DeserializeClass(const json::JSON& inJson)
{
	UActorComponent::DeserializeClass(inJson);

	const json::JSON& propertiesJson = inJson.at("Properties");

	if (!propertiesJson.hasKey("RelativeLocation")
		|| propertiesJson.at("RelativeLocation").JSONType() != json::JSON::Class::Array
		|| propertiesJson.at("RelativeLocation").length() != 3)
	{
		throw std::runtime_error(std::format("{}: RelativeLocation property requires an array of length 3", GetRuntimeClass()->Name));
	}

	if (!propertiesJson.hasKey("RelativeRotation")
		|| propertiesJson.at("RelativeRotation").JSONType() != json::JSON::Class::Array
		|| propertiesJson.at("RelativeRotation").length() != 3)
	{
		throw std::runtime_error(std::format("{}: RelativeRotation property requires an array of length 3", GetRuntimeClass()->Name));
	}

	if (!propertiesJson.hasKey("RelativeScale3D")
		|| propertiesJson.at("RelativeScale3D").JSONType() != json::JSON::Class::Array
		|| propertiesJson.at("RelativeScale3D").length() != 3)
	{
		throw std::runtime_error(std::format("{}: RelativeScale3D property requires an array of length 3", GetRuntimeClass()->Name));
	}


	mRelativeLocation = FVectorFromJson(propertiesJson.at("RelativeLocation"));
	mRelativeRotation = FRotatorFromJson(propertiesJson.at("RelativeRotation"));
	mRelativeScale3D = FVectorFromJson(propertiesJson.at("RelativeScale3D"));
}

FVector USceneComponent::GetRelativeLocation() const
{
	return mRelativeLocation;
}

void USceneComponent::SetRelativeLocation(FVector location)
{
	mRelativeLocation = location;
}

FRotator USceneComponent::GetRelativeRotation() const
{
	return mRelativeRotation;
}

void USceneComponent::SetRelativeRotation(FRotator rotation)
{
	mRelativeRotation = rotation;
}

FVector USceneComponent::GetRelativeScale3D() const
{
	return mRelativeScale3D;
}

void USceneComponent::SetRelativeScale3D(FVector scale)
{
	mRelativeScale3D = scale;
}

FTransform USceneComponent::GetTransformMatrix() const
{
	return FTransform(mRelativeLocation, mRelativeRotation, mRelativeScale3D);
}
