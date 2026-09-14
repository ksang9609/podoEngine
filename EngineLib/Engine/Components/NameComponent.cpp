#include "NameComponent.h"
#include "Core/IO/JsonUtil.h"

IMPLEMENT_CLASS(UNameComponent, UBillboardComponent)

void UNameComponent::Initialize(const FString& nameText, FVector worldPositionOffset, const FFontResource& fontResourceRef)
{
	UBillboardComponent::Initialize(worldPositionOffset, FRotator(), FVector(0));

	mNameText = nameText;
	mFontResourceRef = &fontResourceRef;

	mTextMesh.SetText(mNameText, *mFontResourceRef);
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
	mComponentToWorld = FTransform(worldPosition, FQuat::Identity(), mRelativeScale3D).MakeMatrix();
}

FRenderInfo UNameComponent::makeRenderInfo() const
{
	FRenderInfo renderInfo = UBillboardComponent::makeRenderInfo();
	ERenderFlags renderFlags = renderInfo.eRenderFlags;

	// Remove primitive flags and add billboardtext flags
	renderFlags = renderFlags
		& ~ERenderFlags::RF_Primitive
		& ~ERenderFlags::RF_BoundingBox
		| ERenderFlags::RF_BillboardText;

	renderInfo.eRenderFlags = renderFlags;
	renderInfo.Color = FVector4(1.0f, 1.0f, 1.0f, 1.0f); // White color for name text
	renderInfo.Textmesh = &mTextMesh;

	return renderInfo;
}

void UNameComponent::SetNameText(const FString& nameText)
{
	mNameText = nameText;

	// TODO: Optimize this by updating in the GetRenderInfos function instead of recreating the FTextMesh every time.
	mTextMesh.SetText(mNameText, *mFontResourceRef);
}

UNameComponent::~UNameComponent()
{
}

