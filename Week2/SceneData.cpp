#include "SceneData.h"
#include "Json/json.hpp"

FPrimitiveData::FPrimitiveData()
	: Location(0.f, 0.f, 0.f)
	, Rotation(0.f, 0.f, 0.f)
	, Scale(1.f, 1.f, 1.f)
	, PrimitiveType(EPrimitive::EP_Sphere)
{
}

FPrimitiveData::FPrimitiveData(json::JSON json)
{
	if (!json.hasKey("Location") || !json.hasKey("Rotation") || !json.hasKey("Scale") || !json.hasKey("PrimitiveType"))
	{
		throw std::runtime_error("Invalid JSON format for FPrimitiveData");
		return;
	}

	Location = FVectorFromJson(json["Location"]);
	Rotation = FRotatorFromJson(json["Rotation"]);
	Scale = FVectorFromJson(json["Scale"]);
	PrimitiveType = EPrimitiveFromJson(json["PrimitiveType"]);
}

json::JSON FPrimitiveData::ToJson() const
{
	json::JSON json;
	json["Location"] = FVectorToJson(Location);
	json["Rotation"] = FRotatorToJson(Rotation);
	json["Scale"] = FVectorToJson(Scale);
	json["PrimitiveType"] = EPrimitiveToJson(PrimitiveType);
	return json;
}

FString FPrimitiveData::ToJsonString() const
{
	return FString(ToJson().dump());
}

FSceneData::FSceneData()
	: Version(0)
	, NextUUID(0)
{
}

FSceneData::FSceneData(json::JSON json)
{
	if (!json.hasKey("Version") || !json.hasKey("NextUUID") || !json.hasKey("Primitives"))
	{
		throw std::runtime_error("Invalid JSON format for FSceneData");
		return;
	}

	Version = json["Version"].ToInt();
	NextUUID = json["NextUUID"].ToInt();

	for (const auto& [key, value] : json["Primitives"].ObjectRange())
	{
		uint32 UUID = std::stoul(key);
		FPrimitiveData PrimitiveData(value);
		Primitives.Add(UUID, PrimitiveData);
	}
}

json::JSON FSceneData::ToJson() const
{
	json::JSON json;
	json["Version"] = Version;
	json["NextUUID"] = NextUUID;
	json::JSON primitivesJson = json::JSON::Make(json::JSON::Class::Object);
	for (const auto& [UUID, PrimitiveData] : Primitives)
	{
		primitivesJson[std::to_string(UUID)] = PrimitiveData.ToJson();
	}
	json["Primitives"] = primitivesJson;
	return json;
}

FString FSceneData::ToJsonString() const
{
	return FString(ToJson().dump());
}

json::JSON FVectorToJson(const FVector& Vector)
{
	json::JSON vectorJson = json::JSON::Make(json::JSON::Class::Array);
	vectorJson[0] = Vector.x;
	vectorJson[1] = Vector.y;
	vectorJson[2] = Vector.z;
	return vectorJson;
}

json::JSON FRotatorToJson(const FRotator& Rotator)
{
	json::JSON rotatorJson = json::JSON::Make(json::JSON::Class::Array);
	rotatorJson[0] = Rotator.Pitch;
	rotatorJson[1] = Rotator.Yaw;
	rotatorJson[2] = Rotator.Roll;
	return rotatorJson;
}

json::JSON EPrimitiveToJson(const EPrimitive& Primitive)
{
	switch (Primitive)
	{
	case EPrimitive::EP_Sphere:
		return json::JSON("Sphere");
	case EPrimitive::EP_Cube:
		return json::JSON("Cube");
	case EPrimitive::EP_Triangle:
		return json::JSON("Triangle");
	default:
		throw std::runtime_error("Unknown EPrimitive value");
	}
}

FVector FVectorFromJson(const json::JSON& json)
{
	if (json.JSONType() != json::JSON::Class::Array)
	{
		throw std::runtime_error("Json Array expected for FVector");
	}

	return FVector(json.at(0).ToFloat(), json.at(1).ToFloat(), json.at(2).ToFloat());
}

FRotator FRotatorFromJson(const json::JSON& json)
{
	if (json.JSONType() != json::JSON::Class::Array)
	{
		throw std::runtime_error("Json Array expected for FRotator");
	}

	return FRotator(json.at(0).ToFloat(), json.at(1).ToFloat(), json.at(2).ToFloat());
}

EPrimitive EPrimitiveFromJson(const json::JSON& json)
{
	if (json.JSONType() != json::JSON::Class::String)
	{
		throw std::runtime_error("Json String expected for EPrimitive");
	}
	std::string primitiveStr = json.ToString();
	if (primitiveStr == "Sphere")
	{
		return EPrimitive::EP_Sphere;
	}
	else if (primitiveStr == "Cube")
	{
		return EPrimitive::EP_Cube;
	}
	else if (primitiveStr == "Triangle")
	{
		return EPrimitive::EP_Triangle;
	}
	else
	{
		throw std::runtime_error("Unknown EPrimitive value in JSON");
	}
}
