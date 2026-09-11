#include "SceneComponent.h"

#include <format>

#include "Transform.h"
#include "JsonUtil.h"

IMPLEMENT_CLASS(USceneComponent, UActorComponent);

void USceneComponent::Initialize(FVector location, FRotator rotation, FVector scale3D)
{
	UActorComponent::Initialize();

	mRelativeLocation = location;
	mRelativeRotation = rotation;
	mRelativeScale3D = scale3D;
	mComponentToWorld = FTransform(mRelativeLocation, mRelativeRotation, mRelativeScale3D);
}

USceneComponent::~USceneComponent()
{
}

void USceneComponent::SerializeClass(json::JSON& outJson) const
{
	UActorComponent::SerializeClass(outJson);
	outJson["Properties"]["mRelativeLocation"] = FVectorToJson(mRelativeLocation);
	outJson["Properties"]["mRelativeRotation"] = FRotatorToJson(mRelativeRotation);
	outJson["Properties"]["mRelativeScale3D"] = FVectorToJson(mRelativeScale3D);
}

void USceneComponent::DeserializeClass(const json::JSON& inJson)
{
	UActorComponent::DeserializeClass(inJson);

	const json::JSON& propertiesJson = inJson.at("Properties");

	if (!propertiesJson.hasKey("mRelativeLocation")
		|| propertiesJson.at("mRelativeLocation").JSONType() != json::JSON::Class::Array
		|| propertiesJson.at("mRelativeLocation").length() != 3)
	{
		throw std::runtime_error(std::format("{}: mRelativeLocation property requires an array of length 3", GetRuntimeClass()->Name));
	}

	if (!propertiesJson.hasKey("mRelativeRotation")
		|| propertiesJson.at("mRelativeRotation").JSONType() != json::JSON::Class::Array
		|| propertiesJson.at("mRelativeRotation").length() != 3)
	{
		throw std::runtime_error(std::format("{}: mRelativeRotation property requires an array of length 3", GetRuntimeClass()->Name));
	}

	if (!propertiesJson.hasKey("mRelativeScale3D")
		|| propertiesJson.at("mRelativeScale3D").JSONType() != json::JSON::Class::Array
		|| propertiesJson.at("mRelativeScale3D").length() != 3)
	{
		throw std::runtime_error(std::format("{}: mRelativeScale3D property requires an array of length 3", GetRuntimeClass()->Name));
	}

	mRelativeLocation = FVectorFromJson(propertiesJson.at("mRelativeLocation"));
	mRelativeRotation = FRotatorFromJson(propertiesJson.at("mRelativeRotation"));
	mRelativeScale3D = FVectorFromJson(propertiesJson.at("mRelativeScale3D"));

	mComponentToWorld = FTransform(mRelativeLocation, mRelativeRotation, mRelativeScale3D);
}

FVector USceneComponent::GetRelativeLocation() const
{
	return mRelativeLocation;
}

void USceneComponent::SetRelativeLocation(FVector location)
{
	mRelativeLocation = location;
	mComponentToWorld.SetLocation(location);
}

FRotator USceneComponent::GetRelativeRotation() const
{
	return mRelativeRotation;
}

void USceneComponent::SetRelativeRotation(FRotator rotation)
{
	mRelativeRotation = rotation;
	mComponentToWorld.SetRotation(rotation);
}

void USceneComponent::SetRelativeRotation(FQuat rotation)
{
	mRelativeRotation = rotation.Rotator();
	mComponentToWorld.SetRotation(rotation);
}

FVector USceneComponent::GetRelativeScale3D() const
{
	return mRelativeScale3D;
}

void USceneComponent::SetRelativeScale3D(FVector scale)
{
	mRelativeScale3D = scale;
	mComponentToWorld.SetScale(mRelativeScale3D);
}

FTransform USceneComponent::GetTransformMatrix() const
{
	return mComponentToWorld;
}
