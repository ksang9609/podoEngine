#include "NameComponent.h"
#include "Core/IO/JsonUtil.h"

#include <format>

#include "Engine/Actor.h"

IMPLEMENT_CLASS(UNameComponent, UBillboardComponent)

void UNameComponent::Initialize(const FString& nameText, FVector worldPositionOffset, const FFontResource& fontResourceRef)
{
	UBillboardComponent::Initialize(worldPositionOffset, FRotator(), FVector(0));

	mFontResourceRef = &fontResourceRef;
	mNameText = nameText;
}

void UNameComponent::SerializeClass(json::JSON& outJson) const
{
	UBillboardComponent::SerializeClass(outJson);
	outJson["Properties"]["mNameText"] = mNameText.CStr();
}

void UNameComponent::DeserializeClass(const json::JSON& inJson)
{
	UBillboardComponent::DeserializeClass(inJson);

	const json::JSON& propertiesJson = inJson.at("Properties");
	if (!propertiesJson.hasKey("mNameText") || propertiesJson.at("mNameText").JSONType() != json::JSON::Class::String)
	{
		throw std::runtime_error("UNameComponent: mNameText requires a string");
	}

	mNameText = FString(propertiesJson.at("mNameText").ToString());
	mFontResourceRef = FObjectFactory::GetDefaultFontResource();

	mTextMesh.SetText(mNameText, *mFontResourceRef);
}

void UNameComponent::updateComponentToWorld(const FMatrix& parentTransform)
{
	// NameComponent always located over the actor's world position,
	// so we reuse mRelativeLocation as a world position offset from the actor's world position.

	FVector parentTranslation = parentTransform.GetTranslation();
	FVector worldPosition = parentTranslation + mRelativeLocation;

	if (mParent)
	{
		worldPosition.z = mParent->GetWorldBounds().max.z+0.2f;
	}
	mComponentToWorld = FTransform(worldPosition, FQuat::Identity(), mRelativeScale3D).MakeMatrix();
}

FRenderInfo UNameComponent::makeRenderInfo() const
{
	FRenderInfo renderInfo = UBillboardComponent::makeRenderInfo();
	ERenderFlags renderFlags = renderInfo.eRenderFlags;

	// Remove primitive flags and add billboardtext flags
	renderFlags = renderFlags
		& ~ERenderFlags::RF_Raycastable
		& ~ERenderFlags::RF_Primitive
		& ~ERenderFlags::RF_BoundingBox
		| ERenderFlags::RF_Billboard
		| ERenderFlags::RF_Text;

	renderInfo.eRenderFlags = renderFlags;
	renderInfo.Color = FVector4(1.0f, 1.0f, 1.0f, 1.0f); // White color for name text
	renderInfo.Textmesh = &mTextMesh;

	return renderInfo;
}

void UNameComponent::SetNameText(const FString& nameText)
{
	assert(mOwner);

	FString text = FString(std::format("Name: {}, UUID: {}", nameText, mOwner->UUID));
	mNameText = text;

	// TODO: Optimize this by updating in the GetRenderInfos function instead of recreating the FTextMesh every time.
	mTextMesh.SetText(mNameText, *mFontResourceRef);
}
//
//void UNameComponent::SetNameText(FString&& nameText)
//{//
//	// TODO: Optimize this by updating in the GetRenderInfos function instead of recreating the FTextMesh every time.
//	mTextMesh.SetText(mNameText, *mFontResourceRef);
//}

bool UNameComponent::AttachTo(USceneComponent& parent)
{
	if (!UBillboardComponent::AttachTo(parent))
	{
		return false;
	}

	SetNameText(mOwner->GetName().ToString());
	return true;
}

UNameComponent::~UNameComponent()
{
}

