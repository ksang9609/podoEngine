#pragma once

#include "BillboardComponent.h"

class UNameComponent : public UBillboardComponent
{
	DECLARE_OBJECT(UNameComponent, UBillboardComponent)

public:
	UNameComponent() = default;

	void Initialize(const FString& nameText, FVector worldPositionDelta);

private:
	FString mNameText;
};
