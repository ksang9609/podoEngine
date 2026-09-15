#pragma once

#include "Core/Math/Color.h"

#include "SceneComponent.h"

class UPrimitiveComponent : public USceneComponent
{
	DECLARE_OBJECT(UPrimitiveComponent, USceneComponent)
public:
	UPrimitiveComponent();

	//void Initialize(GraphicsManager* graphicsManager, EPrimitive ePrimitive);
	//void Initialize(GraphicsManager* graphicsManager, EPrimitive ePrimitive, FVector location, FRotator rotation, FVector scale3D);

	void Initialize(EPrimitive ePrimitive);
	void Initialize(EPrimitive ePrimitive, FVector location, FRotator rotation, FVector scale3D);
	void Initialize(EPrimitive ePrimitive, FVector location, FRotator rotation, FVector scale3D, bool bUseTexture);

	virtual ~UPrimitiveComponent();

	virtual void SerializeClass(json::JSON& outJson) const override;
	virtual void DeserializeClass(const json::JSON& inJson) override;

	virtual FBoundingBox GetWorldBounds() const override;

	//virtual void Render();
	void Update(float deltaTime, TArray<FRenderInfo>* outRenderInfos) override;
	void GetRenderInfos(TArray<FRenderInfo>* outRenderInfos) const override final;
	void SetUseTexture(bool value) { mbUseTexture = value; }
	bool GetUseTexture() const { return mbUseTexture; }

	const FLinearColor& GetColor() const { return mColor; }
	void SetColor(const FLinearColor& color) { mColor = color; }

protected:
	virtual FRenderInfo makeRenderInfo() const;
	//GraphicsManager* mGraphicsManager;
	EPrimitive mePrimitive;
	FLinearColor mColor{ 1.f, 1.f, 1.f, 1.f };

	FBoundingBox mLocalBounds{};
	FBoundingBox mWocalBounds{};

	bool mbUseTexture = false;
	bool mbShowBoundingBox = true;
};


