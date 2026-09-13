#include "SceneComponent.h"

#include <format>

#include "Transform.h"
#include "JsonUtil.h"
#include "Actor.h"

IMPLEMENT_CLASS(USceneComponent, UActorComponent);

void USceneComponent::Initialize(FVector location, FRotator rotation, FVector scale3D)
{
	UActorComponent::Initialize();

	mRelativeLocation = location;
	mRelativeRotation = rotation;
	mRelativeScale3D = scale3D;
	updateComponentToWorld();
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

	updateComponentToWorld();
}

// Attach this component to a parent scene component
bool USceneComponent::AttachTo(USceneComponent& parent)
{
	if (&parent == this)
	{
		return false;
	}
	if (mParent == &parent)
	{
		return true; // Already attached to the same parent
	}
	if (parent.isChildOf(*this))
	{
		return false; // Prevent circular attachment
	}

	if (mOwner && mOwner != parent.GetOwner())
	{
		mOwner->RemoveComponent(UUID);

		// Add this component to the owner actor's component list
		parent.GetOwner()->AddComponent(this);
	}

	if (mParent && mParent != &parent)
	{
		mParent->RemoveChild(*this);
	}

	mParent = &parent;
	parent.mChildren.Add(this);

	updateComponentToWorld(parent.GetTransformMatrix());

	return true;
}

bool USceneComponent::RemoveChild(USceneComponent& child)
{
	if (&child == this)
	{
		return false; // Cannot remove self
	}
	if (child.mParent != this)
	{
		return false; // The specified child is not a child of this component
	}

	if (!mChildren.Remove(&child))
	{
		return false;
	}

	child.mParent = nullptr;

	// Reset the child's transform to world space
	child.updateComponentToWorld();
	return true;
}

void USceneComponent::DetachFromParent()
{
	if (mParent)
	{
		mParent->RemoveChild(*this);
	}
}

void  USceneComponent::DetachAllChildren()
{
	while (mChildren.Num() > 0)
	{
		RemoveChild(*mChildren[0]);
	}
}

FVector USceneComponent::GetRelativeLocation() const
{
	return mRelativeLocation;
}

void USceneComponent::SetRelativeLocation(FVector location)
{
	mRelativeLocation = location;
	updateComponentToWorld();
}

FRotator USceneComponent::GetRelativeRotation() const
{
	return mRelativeRotation;
}

void USceneComponent::SetRelativeRotation(FRotator rotation)
{
	mRelativeRotation = rotation;
	updateComponentToWorld();
}

void USceneComponent::SetRelativeRotation(FQuat rotation)
{
	mRelativeRotation = rotation.Rotator();
	updateComponentToWorld();
}

FVector USceneComponent::GetRelativeScale3D() const
{
	return mRelativeScale3D;
}

void USceneComponent::SetRelativeScale3D(FVector scale)
{
	mRelativeScale3D = scale;
	updateComponentToWorld();
}

FMatrix USceneComponent::GetTransformMatrix() const
{
	return mComponentToWorld;
}

FTransform USceneComponent::GetRelativeTransform() const
{
	return FTransform(mRelativeLocation, mRelativeRotation, mRelativeScale3D);
}

void USceneComponent::SetRelativeTransform(const FTransform& transform)
{
	mRelativeLocation = transform.GetLocation();
	mRelativeRotation = transform.GetRotator();
	mRelativeScale3D = transform.GetScale();
	updateComponentToWorld();
}

void USceneComponent::updateComponentToWorld(const FMatrix& parentTransform)
{
	mComponentToWorld = FTransform(mRelativeLocation, mRelativeRotation, mRelativeScale3D).MakeMatrix() * parentTransform;

	for (USceneComponent* child : mChildren)
	{
		child->updateComponentToWorld(mComponentToWorld);
	}
}

void USceneComponent::updateComponentToWorld()
{
	updateComponentToWorld(mParent ? mParent->GetTransformMatrix() : FMatrix::Identity);
}


bool USceneComponent::isChildOf(const USceneComponent& component) const
{
	const USceneComponent* current = mParent;
	while (current)
	{
		if (current == &component)
		{
			return true;
		}
		current = current->mParent;
	}
	return false;
}
