#pragma once

#include "ActorComponent.h"
#include "Rendering/GraphicsManager.h"

#include "Core/Math/Vector.h"
#include "Core/Container/TArray.h"

#include <span>
#include "Core/Object/PropertyInfo.h"

class FTransform;

class USceneComponent : public UActorComponent
{
	DECLARE_OBJECT(USceneComponent, UActorComponent)
public:
	USceneComponent() = default;

	void Initialize(FVector location, FRotator rotation, FVector scale3D);
	virtual ~USceneComponent();

	virtual void SerializeClass(json::JSON& outJson) const override;
	virtual void DeserializeClass(const json::JSON& inJson) override;

	virtual bool AttachTo(USceneComponent& parent);
	virtual bool RemoveChild(USceneComponent& child);
	virtual void DetachFromParent();
	virtual void DetachAllChildren();
	int32 GetParentUUID() const;
	int32 GetSerializedParentUUID() const;

	FVector GetRelativeLocation() const;
	void SetRelativeLocation(FVector location);

	FRotator GetRelativeRotation() const;
	void SetRelativeRotation(FRotator rotation);
	void SetRelativeRotation(FQuat rotation);

	FVector GetRelativeScale3D() const;
	void SetRelativeScale3D(FVector scale);

	FTransform GetRelativeTransform() const;
	void SetRelativeTransform(const FTransform& transform);

	FMatrix GetTransformMatrix() const;

	//test
	static std::span<const FPropertyInfo> GetDeclaredProperties();
	std::span<const FPropertyInfo> DeclaredProperties{};

protected:
	FVector mRelativeLocation;
	FRotator mRelativeRotation;
	FVector mRelativeScale3D;

	FMatrix mComponentToWorld;


	// References of parent component and child components.
	// The ownership of child components is managed by the actor, not by the parent component.
	USceneComponent* mParent = nullptr;
	TArray<USceneComponent*> mChildren;
	int32 mSerializedParentUUID = -1;

	virtual void updateComponentToWorld(const FMatrix& parentTransform);
	virtual void updateComponentToWorld();
	bool isChildOf(const USceneComponent& component) const;
};

