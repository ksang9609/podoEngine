#pragma once

#include "ThirdParty/Json/json.hpp"
#include "Core/IO/JsonUtil.h"
#include "Core/Name.h"


template<typename T>
struct TPropertyJsonSerializer;

template<>
struct TPropertyJsonSerializer<int32>
{
	static void Serialize(json::JSON& OutJson,
		const char* Key, const int32& Value)
	{
		OutJson[Key] = Value;
	}

	static void Deserialize(const json::JSON& InJson,
		const char* Key, int32& OutValue)
	{
		if (!InJson.hasKey(Key))
		{
			throw std::runtime_error("Missing int32 property");
		}

		const json::JSON& Value = InJson.at(Key);

		// 이 부분은 네 JSON 라이브러리의
		// 숫자 타입 enum 이름에 맞춰야 함.
		if (Value.JSONType() != json::JSON::Class::Integral)
		{
			throw std::runtime_error("Property requires int32");
		}

		OutValue = static_cast<int32>(Value.ToInt());
	}
};

template<>
struct TPropertyJsonSerializer<FString>
{
	static void Serialize(
		json::JSON& OutJson,
		const char* Key,
		const FString& Value)
	{
		OutJson[Key] = Value.CStr();
	}

	static void Deserialize(
		const json::JSON& InJson,
		const char* Key,
		FString& OutValue)
	{
		if (!InJson.hasKey(Key))
		{
			throw std::runtime_error("Missing FString property");
		}

		const json::JSON& Value = InJson.at(Key);

		if (Value.JSONType() != json::JSON::Class::String)
		{
			throw std::runtime_error("Property requires string");
		}

		OutValue = FString(Value.ToString());
	}
};

template<>
struct TPropertyJsonSerializer<FVector>
{
	static void Serialize(
		json::JSON& OutJson,
		const char* Key,
		const FVector& Value)
	{
		OutJson[Key] = FVectorToJson(Value);
	}

	static void Deserialize(
		const json::JSON& InJson,
		const char* Key,
		FVector& OutValue)
	{
		if (!InJson.hasKey(Key))
		{
			throw std::runtime_error("Missing FVector property");
		}

		const json::JSON& Value = InJson.at(Key);

		if (Value.JSONType() != json::JSON::Class::Array)
		{
			throw std::runtime_error("Property requires FVector");
		}

		OutValue = FVectorFromJson(Value);
	}
};

template<>
struct TPropertyJsonSerializer<FRotator>
{
	static void Serialize(
		json::JSON& OutJson,
		const char* Key,
		const FRotator& Value)
	{
		OutJson[Key] = FRotatorToJson(Value);
	}

	static void Deserialize(
		const json::JSON& InJson,
		const char* Key,
		FRotator& OutValue)
	{
		if (!InJson.hasKey(Key))
		{
			throw std::runtime_error("Missing FRotator property");
		}

		const json::JSON& Value = InJson.at(Key);

		if (Value.JSONType() != json::JSON::Class::Array)
		{
			throw std::runtime_error("Property requires FRotator");
		}

		OutValue = FRotatorFromJson(Value);
	}
};

template<>
struct TPropertyJsonSerializer<EPrimitive>
{
	static void Serialize(
		json::JSON& OutJson,
		const char* Key,
		const EPrimitive& Value)
	{
		OutJson[Key] = EPrimitiveToJson(Value);
	}

	static void Deserialize(
		const json::JSON& InJson,
		const char* Key,
		EPrimitive& OutValue)
	{
		if (!InJson.hasKey(Key))
		{
			throw std::runtime_error("Missing EPrimitive property");
		}

		const json::JSON& Value = InJson.at(Key);

		if (Value.JSONType() != json::JSON::Class::String)
		{
			throw std::runtime_error("Property requires EPrimitive");
		}

		OutValue = EPrimitiveFromJson(Value);
	}
};

template<>
struct TPropertyJsonSerializer<FName>
{
	static void Serialize(
		json::JSON& OutJson,
		const char* Key,
		const FName& Value)
	{
		OutJson[Key] = Value.ToString();
	}

	static void Deserialize(
		const json::JSON& InJson,
		const char* Key,
		FName& OutValue)
	{
		if (!InJson.hasKey(Key))
		{
			throw std::runtime_error("Missing FName property");
		}

		const json::JSON& Value = InJson.at(Key);

		if (Value.JSONType() != json::JSON::Class::String)
		{
			throw std::runtime_error("Property requires FName");
		}

		const FString LoadedName(Value.ToString());
		OutValue = FName(LoadedName);
	}
};
