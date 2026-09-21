#pragma once

#include "Core/Core.h"
#include "ThirdParty/Json/json.hpp"

struct FJsonArchiveData
{
	uint32 NextUUID = 0;
	json::JSON WorldJson;
};

class FJsonArchive
{
public:
	static json::JSON Serialize(const json::JSON& worldJson, uint32 nextUUID);
	static FJsonArchiveData Deserialize(const json::JSON& archiveJson);
};
