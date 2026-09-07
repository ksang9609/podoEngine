#pragma once

#include "Object.h"

struct FRenderInfo;

class UActorComponent : public UObject
{
	REFLECT_CLASS(UActorComponent, UObject)
public:
	UActorComponent();
	virtual ~UActorComponent();

	 //Todo: Make as pure class
	virtual void Update() {};
	//virtual void Render();

	virtual void GetRenderInfos(TArray<FRenderInfo>* outRenderInfos);
};

