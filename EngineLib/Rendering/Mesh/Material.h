// EngineLib/Rendering/Mesh/Material.h

#pragma once

#include <memory>

#include "Core/Core.h"
#include "Core/Name.h"
#include "Core/Math/Vector.h"

// GPU 객체를 직접 소유하지 않는다.
// 텍스처 경로와 셰이더 파라미터만 나타내는 가벼운 값 타입이다.
struct FMaterial
{
	FVector AmbientColor = FVector(0.0f, 0.0f, 0.0f);
	FVector DiffuseColor = FVector(1.0f, 1.0f, 1.0f);
	FVector SpecularColor = FVector(0.0f, 0.0f, 0.0f);

	float SpecularExponent = 0.0f;
	float Opacity = 1.0f;

	FName DiffuseTexture;
	FName NormalTexture;
	FName SpecularTexture;
};

// 슬롯 이름은 OBJ의 usemtl과 연결하기 위해 매시가 소유한다.
struct FMaterialSlot
{
	FString Name;
	FMaterial DefaultMaterial;
};

class UMaterial : public UObject
{
	DECLARE_OBJECT(UMaterial, UObject)
	DECLARE_SERIALIZATION()
public:

private:
	std::unique_ptr<FMaterial> mMaterial;
};
