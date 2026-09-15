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
	mElapsedTime = 0.0f;
	mCurrentFrameIndex = 0;

	// Initialize the sub UV mesh
	mSubUVMesh.UpdateMesh(mNumRows, mNumCols, 0);

	// Call the base class Initialize
	UBillboardComponent::Initialize(location, rotation, scale3D);
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
	UBillboardComponent::Update(deltaTime, outRenderInfos);

	if (!mbLooping && mElapsedTime >= 1.0f / mPlayRate)
	{
		return; // Stop updating if not looping and the animation has finished
	}
	
	mElapsedTime += deltaTime;
	float frameDuration = mFrameDuration / mPlayRate;
	if (mElapsedTime >= frameDuration)
	{
		mElapsedTime -= frameDuration;
		mCurrentFrameIndex++;
		if (mCurrentFrameIndex >= mNumRows * mNumCols)
		{
			if (mbLooping)
			{
				mCurrentFrameIndex = 0;
			}
			else
			{
				// Clamp to the last frame if not looping
				mCurrentFrameIndex = mNumRows * mNumCols - 1;
			}
		}
		mSubUVMesh.UpdateMesh(mNumRows, mNumCols, mCurrentFrameIndex);
	}
}

FRenderInfo UParticleSubUVComponent::makeRenderInfo() const
{
	FRenderInfo renderInfo = UBillboardComponent::makeRenderInfo();
	renderInfo.SubUVMesh = &mSubUVMesh;

	renderInfo.eRenderFlags =
		ERenderFlags::RF_Billboard |
		ERenderFlags::RF_Particle;
	renderInfo.Color = FVector4(1.0f, 1.0f, 1.0f, 1.0f); // White color for the particle
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
