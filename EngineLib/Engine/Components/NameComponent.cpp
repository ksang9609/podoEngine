#include "NameComponent.h"

#include <format>

#include "Core/IO/JsonUtil.h"
#include "Core/Math/Matrix.h"
#include "Core/Math/Transform.h"
#include "Engine/Actor.h"

IMPLEMENT_CLASS_WITH_PROPERTIES(UNameComponent, UBillboardComponent);
IMPLEMENT_SERIALIZATION(UNameComponent, UBillboardComponent,
	{
		mFontResourceRef = FObjectFactory::GetDefaultFontResource();
		mTextMesh.SetUnicodeText(mNameText, *mFontResourceRef, 0.2f);
	}
)

void UNameComponent::Initialize(const FString& nameText, FVector worldPositionOffset, const FFontResource& fontResourceRef)
{
	UBillboardComponent::Initialize(worldPositionOffset, FRotator(), FVector(1));

	mFontResourceRef = &fontResourceRef;
	mNameText = nameText;
	mColor = FLinearColor(1.f, 1.f, 1.f, 1.f); // Set default color to white
}

void UNameComponent::updateComponentToWorld(const FMatrix& parentTransform) const
{
	// NameComponent always located over the actor's world position,
	// so we reuse mRelativeLocation as a world position offset from the actor's world position.

	FVector parentTranslation = parentTransform.GetTranslation();
	FVector worldPosition = parentTranslation + mRelativeLocation;

	if (mParent)
	{
		if (const UPrimitiveComponent* primitiveParent = mParent->Cast<UPrimitiveComponent>())
		{
			// Set the world position to be above the parent's bounding box
			const FBoundingBox parentBounds = primitiveParent->CalculateWorldBounds(parentTransform);

			worldPosition.z = parentBounds.max.z + mRelativeLocation.z;
		}
	}
	mComponentToWorld = FTransform(worldPosition, FQuat::Identity(), mRelativeScale3D).MakeMatrix();

	mbTransformDirty = false;
	mbRenderInfoDirty = true;
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
	renderInfo.Textmesh = &mTextMesh;

	return renderInfo;
}

void UNameComponent::updateRenderInfo()
{
	UBillboardComponent::updateRenderInfo();
	ERenderFlags renderFlags = mRenderInfo.eRenderFlags;

	// Remove primitive flags and add billboardtext flags
	renderFlags = renderFlags
		& ~ERenderFlags::RF_Raycastable
		& ~ERenderFlags::RF_Primitive
		& ~ERenderFlags::RF_BoundingBox
		| ERenderFlags::RF_Billboard
		| ERenderFlags::RF_Text;

	mRenderInfo.eRenderFlags = renderFlags;
	mRenderInfo.Textmesh = &mTextMesh;
}

void UNameComponent::SetNameText(const FString& nameText)
{
	assert(mOwner);

	FString text = FString(std::format("Name: {}, UUID: {}", nameText, mOwner->UUID));
	mNameText = text;

	// TODO: Optimize this by updating in the GetRenderInfos function instead of recreating the FTextMesh every time.
	mTextMesh.SetText(mNameText, *mFontResourceRef);

	mbRenderInfoDirty = true;
}
//
//void UNameComponent::SetNameText(FString&& nameText)
//{//
//	// TODO: Optimize this by updating in the GetRenderInfos function instead of recreating the FTextMesh every time.
//	mTextMesh.SetText(mNameText, *mFontResourceRef);
//}

void UNameComponent::SetUnicodeNameText(const FString& nameText)
{
	assert(mOwner);

	FString text = FString(std::format("Name: {}, UUID: {}", nameText, mOwner->UUID));
	mNameText = text;

	// 내부에서 FontRenderMode를 MSDF로 설정
	mTextMesh.SetUnicodeText(mNameText, *mFontResourceRef, 0.2f);

	mbRenderInfoDirty = true;
}

bool UNameComponent::AttachTo(USceneComponent& parent)
{
	if (!UBillboardComponent::AttachTo(parent))
	{
		return false;
	}

	//SetNameText(mOwner->GetName().ToString());
	SetUnicodeNameText(mOwner->GetName().ToString());
	return true;
}

UNameComponent::~UNameComponent()
{
}

std::span<const FPropertyInfo> UNameComponent::GetDeclaredProperties()
{
	static const FPropertyInfo Properties[] =
	{
		REFLECT_PROPERTY(
			UNameComponent,
			mNameText),
	};

	return Properties;
}
