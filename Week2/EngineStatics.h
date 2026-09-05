#pragma once

#include "Core.h"

class UEngineStatics
{
public:
	static uint32 GenerateUUID();

private:
	static uint32 msNextUUID;
};

inline uint32 UEngineStatics::GenerateUUID()
{
	return msNextUUID++;
}
