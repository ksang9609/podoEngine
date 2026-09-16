#include "ParticleSubUVComponent.h"

IMPLEMENT_CLASS_WITH_PROPERTIES(UParticleSubUVComponent, UBillboardComponent);
IMPLEMENT_SERIALIZATION(UParticleSubUVComponent, UBillboardComponent,
	{ mSubUVMesh.UpdateMesh(mNumRows, mNumCols, 0); })

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

	mColor = FLinearColor(1.f, 1.f, 1.f, 0.2f); // Set default color to white
}

void UParticleSubUVComponent::Update(float deltaTime, TArray<FRenderInfo>* outRenderInfos)
{
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
	return renderInfo;
}

std::span<const FPropertyInfo> UParticleSubUVComponent::GetDeclaredProperties()
{
	static const FPropertyInfo Properties[] =
	{
		REFLECT_PROPERTY(
			UParticleSubUVComponent,
			mNumRows),
		REFLECT_PROPERTY(
			UParticleSubUVComponent,
			mNumCols),
		REFLECT_PROPERTY(
			UParticleSubUVComponent,
			mbLooping),
		REFLECT_PROPERTY(
			UParticleSubUVComponent,
			mPlayRate),
		REFLECT_PROPERTY(
			UParticleSubUVComponent,
			mFrameDuration)
	};

	return Properties;
}
