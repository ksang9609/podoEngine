#pragma once

#include "ActorComponent.h"
#include "GraphicsManager.h"

#include "Vector.h"
#include "TArray.h"

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

	bool AttachTo(USceneComponent& parent);
	bool RemoveChild(USceneComponent& child);

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

protected:
	FVector mRelativeLocation;
	FRotator mRelativeRotation;
	FVector mRelativeScale3D;

	FMatrix mComponentToWorld;

	// References of child components.
	// The ownership of child components is managed by the actor, not by the parent component.
	TArray<USceneComponent*> mChildren;

	void updateComponentToWorld(const FMatrix& parentTransform);
};

