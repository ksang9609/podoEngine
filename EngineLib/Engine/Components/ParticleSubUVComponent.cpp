#include "ParticleSubUVComponent.h"

IMPLEMENT_CLASS_WITH_PROPERTIES(UParticleSubUVComponent, UBillboardComponent);

void UParticleSubUVComponent::Initialize(FVector location, FRotator rotation, FVector scale3D,
	uint32 numRows, uint32 numCols,
	bool bLooping, float playRate, float frameDuration)
{
	mNumRows = numRows;
	mNumCols = numCols;
	mbLooping = bLooping;
	mPlayRate = playRate;
	mFrameDuration = frameDuration;

	mElapsedFrameRatio = 0.0f;
	mCurrentFrameIndex = 0;
	mbIsFinished = false;

	// Initialize the sub UV mesh
	mSubUVMesh.UpdateMesh(mNumRows, mNumCols, 0);

	// Call the base class Initialize
	UBillboardComponent::Initialize(location, rotation, scale3D);

	mColor = FLinearColor(1.f, 1.f, 1.f, 0.2f); // Set default color to white
	mBlendStateType = EBlendStateType::BST_Additive; // Set default blend state to additive
}

void UParticleSubUVComponent::SerializeClass(json::JSON& outJson) const
{
	UBillboardComponent::SerializeClass(outJson);

	for (const FPropertyInfo& Property : ClassInfo.DeclaredProperties)
	{
		Property.Serialize(
			Property,
			this,
			outJson["Properties"]);
	}
}

void UParticleSubUVComponent::DeserializeClass(const json::JSON& inJson)
{
	UBillboardComponent::DeserializeClass(inJson);
	const json::JSON& propertiesJson = inJson.at("Properties");

	for (const FPropertyInfo& Property : ClassInfo.DeclaredProperties)
	{
		Property.Deserialize(
			Property,
			this,
			propertiesJson);
	}

	mSubUVMesh.UpdateMesh(mNumRows, mNumCols, 0);
}


void UParticleSubUVComponent::Update(float deltaTime, TArray<FRenderInfo>* outRenderInfos)
{
	bool restarted = false;

	if (mbIsFinished && mbLooping)
	{
		mbIsFinished = false;
		mCurrentFrameIndex = 0;
		mElapsedFrameRatio = 0;

		mSubUVMesh.UpdateMesh(mNumRows, mNumCols, mCurrentFrameIndex);
		restarted = true;
	}

	if (!mbIsFinished && !restarted &&
		mNumRows > 0 && mNumCols > 0 && mFrameDuration > 0.0f)
	{
		const uint32 totalFrames = mNumRows * mNumCols;
		const uint32 previousFrameIndex = mCurrentFrameIndex;

		if (deltaTime > 0.0f && mPlayRate > 0.0f)
		{
			mElapsedFrameRatio += (deltaTime * mPlayRate) / mFrameDuration;
		}

		while (mElapsedFrameRatio >= 1.0f)
		{
			mElapsedFrameRatio -= 1.0f;

			if (mCurrentFrameIndex + 1 < totalFrames)
			{
				++mCurrentFrameIndex;
			}
			else if (mbLooping)
			{
				mCurrentFrameIndex = 0;
			}
			else
			{
				mbIsFinished = true;
				mElapsedFrameRatio = 0.0f;

				break;
			}
		}

		if (mCurrentFrameIndex != previousFrameIndex)
		{
			mSubUVMesh.UpdateMesh(mNumRows, mNumCols, mCurrentFrameIndex);
		}
	}

	UBillboardComponent::Update(deltaTime, outRenderInfos);
}

FRenderInfo UParticleSubUVComponent::makeRenderInfo() const
{
	FRenderInfo renderInfo = UBillboardComponent::makeRenderInfo();
	renderInfo.SubUVMesh = &mSubUVMesh;

	renderInfo.eRenderFlags =
		ERenderFlags::RF_Raycastable |
		ERenderFlags::RF_Billboard |
		ERenderFlags::RF_Particle;

	renderInfo.Color = mbIsFinished
		? FLinearColor(1.f, 1.f, 1.f, 0.0f) // Fully transparent if finished
		: mColor; // Use the component's color if not finished

	renderInfo.BlendStateType = mBlendStateType;
	return renderInfo;
}

std::span<const FPropertyInfo> UParticleSubUVComponent::GetDeclaredProperties()
{
	static const FPropertyInfo Properties[] =
	{
		REFLECT_PROPERTY(
			UParticleSubUVComponent,
			mNumRows,
			"mNumRows"),
		REFLECT_PROPERTY(
			UParticleSubUVComponent,
			mNumCols,
			"mNumCols"),
		REFLECT_PROPERTY(
			UParticleSubUVComponent,
			mbLooping,
			"mbLooping"),
		REFLECT_PROPERTY(
			UParticleSubUVComponent,
			mPlayRate,
			"mPlayRate"),
		REFLECT_PROPERTY(
			UParticleSubUVComponent,
			mFrameDuration,
			"mFrameDuration")
	};

	return Properties;
}
