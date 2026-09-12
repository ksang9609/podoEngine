
#include "PrimitiveComponent.h"

#include <format>

#include "RenderInfo.h"
#include "enum.h"
#include "JsonUtil.h"
#include "Console.h"
#include "Actor.h"

IMPLEMENT_CLASS(UPrimitiveComponent, USceneComponent);

UPrimitiveComponent::UPrimitiveComponent()
{
}

/*
void UPrimitiveComponent::Initialize(GraphicsManager* graphicsManager, EPrimitive ePrimitive, FVector location, FRotator rotation, FVector scale3D)
{
	USceneComponent::Initialize(location, rotation, scale3D);

	mGraphicsManager = graphicsManager;
	mePrimitive = ePrimitive;
}
*/

void UPrimitiveComponent::Initialize(EPrimitive ePrimitive)
{
	Initialize(ePrimitive, FVector(0.f, 0.f, 0.f), FRotator(0.f, 0.f, 0.f), FVector(0.f, 0.f, 0.f));
}

void UPrimitiveComponent::Initialize(EPrimitive ePrimitive, FVector location, FRotator rotation, FVector scale3D)
{
	USceneComponent::Initialize(location, rotation, scale3D);

	mePrimitive = ePrimitive;
}

void UPrimitiveComponent::Initialize(EPrimitive ePrimitive, FVector location, FRotator rotation, FVector scale3D, bool bUseTexture)
{
	USceneComponent::Initialize(location, rotation, scale3D);
	mePrimitive = ePrimitive;
	mbUseTexture = bUseTexture;
}

UPrimitiveComponent::~UPrimitiveComponent()
{
}

void UPrimitiveComponent::SerializeClass(json::JSON& outJson) const
{
	USceneComponent::SerializeClass(outJson);
	outJson["Properties"]["mePrimitiveType"] = EPrimitiveToJson(mePrimitive);
}

void UPrimitiveComponent::DeserializeClass(const json::JSON& inJson)
{
	USceneComponent::DeserializeClass(inJson);

	const json::JSON& propertiesJson = inJson.at("Properties");
	if (!propertiesJson.hasKey("mePrimitiveType") || propertiesJson.at("mePrimitiveType").JSONType() != json::JSON::Class::String)
	{
		throw std::runtime_error(std::format("{}: mePrimitiveType property requires a string", GetRuntimeClass()->Name));
	}

	mePrimitive = EPrimitiveFromJson(propertiesJson.at("mePrimitiveType"));
}

void UPrimitiveComponent::Update(TArray<FRenderInfo>* outRenderInfos)
{
	// Todo: Update coordinates here
	{
		//UE_LOG("Primitive selected");
	}

	GetRenderInfos(outRenderInfos);
}

void UPrimitiveComponent::GetRenderInfos(TArray<FRenderInfo>* outRenderInfos) const
{
	assert(outRenderInfos);

	outRenderInfos->Add(makeRenderInfo());
}

FRenderInfo UPrimitiveComponent::makeRenderInfo() const
{
	ERenderFlags renderFlags = mbUseTexture
		? ERenderFlags::RF_TexturedPrimitive
		: ERenderFlags::RF_SimplePrimitive;

	if (mbShowBoundingBox)
	{
		renderFlags = renderFlags | ERenderFlags::RF_BoundingBox;
	}

	return {
		mePrimitive,
		GetTransformMatrix(),
		{ mOwner->UUID, mOwner->InternalIndex },
		FVector4(0, 0, 0, 0),
		renderFlags,
		mbUseTexture
	};
}

/*
void UPrimitiveComponent::Render(FStruct)
{
	// Todo: Fix renderer
	mGraphicsManager->Render(GetTransformMatrix(), mePrimitive);
}
*/


