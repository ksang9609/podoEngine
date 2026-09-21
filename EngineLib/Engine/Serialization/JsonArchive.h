#pragma once

#include "Core/Core.h"
#include "ThirdParty/Json/json.hpp"

class UWorld;

class FJsonArchive
{
public:
	static json::JSON SerializeWorld(const UWorld& world);
	static UWorld* DeserializeWorld(const json::JSON& inJson);
};
