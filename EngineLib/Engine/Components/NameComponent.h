#pragma once

#include <functional>

#include "Rendering/TextMesh.h"
#include "Rendering/FontResource.h"

#include "BillboardComponent.h"
#include "Core/Object/ObjectFactory.h"

class UNameComponent : public UBillboardComponent
{
	DECLARE_OBJECT(UNameComponent, UBillboardComponent)

public:
	UNameComponent() = default;

	virtual ~UNameComponent();

	void Initialize(const FString& nameText, FVector worldPositionOffset, const FFontResource& fontResourceRef);
	void SetNameText(const FString& nameText);
	//void SetNameText(FString&& nameText);

	virtual bool AttachTo(USceneComponent& parent) override;

	void SerializeClass(json::JSON& outJson) const override;
	void DeserializeClass(const json::JSON& inJson) override;

protected:
	// NameComponent always located over the actor's world position,
	// so we reuse mRelativeLocation as a world position offset from the actor's world position.
	// FVector mRelativeLocation

	FString mNameText;

	FTextMesh mTextMesh;
	const FFontResource* mFontResourceRef;

	virtual void updateComponentToWorld(const FMatrix& parentTransform) override;
	//virtual void updateComponentToWorld() override;

	virtual FRenderInfo makeRenderInfo() const override;
};
