#pragma once

#include <span>

#include "Core/Math/Color.h"
#include "Rendering/RenderInfo.h"

#include "SceneComponent.h"

class UPrimitiveComponent : public USceneComponent
{
	DECLARE_OBJECT(UPrimitiveComponent, USceneComponent)
	DECLARE_SERIALIZATION()

public:
	UPrimitiveComponent();

	void Initialize(EPrimitive ePrimitive);
	void Initialize(EPrimitive ePrimitive, FVector location, FRotator rotation, FVector scale3D);
	void Initialize(EPrimitive ePrimitive, FVector location, FRotator rotation, FVector scale3D, bool bUseTexture);

	virtual ~UPrimitiveComponent();

	virtual FBoundingBox GetWorldBounds() const override;
	virtual FBoundingBox CalculateWorldBounds(const FMatrix& worldTransform) const;

	void Update(float deltaTime, TArray<const FRenderInfo*>& outRenderInfos) override;
	void GetRenderInfos(TArray<const FRenderInfo*>& outRenderInfos) override final;
	void SetUseTexture(bool value) { mbUseTexture = value; mbRenderInfoDirty = true; }
	bool GetUseTexture() const { return mbUseTexture; }

	const FLinearColor& GetColor() const { return mColor; }
	void SetColor(const FLinearColor& color) { mColor = color; mbRenderInfoDirty = true; }

	static std::span<const FPropertyInfo> GetDeclaredProperties();

protected:
	virtual FRenderInfo makeRenderInfo() const;
	virtual void updateRenderInfo();

	EPrimitive mePrimitive;
	FLinearColor mColor{ 1.f, 1.f, 1.f, 1.f };

	FBoundingBox mLocalBounds{};
	FBoundingBox mWocalBounds{};

	bool mbUseTexture = false;
	bool mbShowBoundingBox = true;

	FRenderInfo mRenderInfo = {};
};


