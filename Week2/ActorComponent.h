#pragma once

#include "Object.h"

class UActorComponent : public UObject
{
	REFLECT_CLASS(UActorComponent, UObject)
public:
	UActorComponent();
	virtual ~UActorComponent();

	// Todo: Make as pure class
	virtual void Update();
	virtual void Render();
};

