#pragma once

#include "BillboardComponent.h"

#include "Rendering/SubUVMesh.h"

class UParticleSubUVComponent : public UBillboardComponent
{
	DECLARE_OBJECT(UParticleSubUVComponent, UBillboardComponent)

public:
	UParticleSubUVComponent() = default;

	void Initialize(FVector location, FRotator rotation, FVector scale3D,
		uint32 numRows, uint32 numCols,
		bool bLooping = true, float playRate = 1.0f, float frameDuration = 1.0f);

	virtual void Update(float deltaTime, TArray<FRenderInfo>* outRenderInfos) override;

	void SerializeClass(json::JSON& outJson) const override;
	void DeserializeClass(const json::JSON& inJson) override;

	static std::span<const FPropertyInfo> GetDeclaredProperties();

	bool IsLooping() const { return mbLooping; }
	void SetLooping(bool bLooping) { mbLooping = bLooping; }

	float GetPlayRate() const { return mPlayRate; }
	void SetPlayRate(float playRate) { mPlayRate = playRate; }

private:
	FSubUVMesh mSubUVMesh;

	/* Particle texture rows and columns */
	uint32 mNumRows = 1;
	uint32 mNumCols = 1;

	bool mbLooping = true;
	float mPlayRate = 1.0f; // Play speed multiplier
	float mFrameDuration = 1.0f; // Duration of each frame in seconds

	/* Internal State */
	float mElapsedTime = 0.0f;
	uint32 mCurrentFrameIndex = 0;

	virtual FRenderInfo makeRenderInfo() const override;
};
