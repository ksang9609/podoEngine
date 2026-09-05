#include "pch.h"

#include "Actor.h"
#include "SceneComponent.h"

namespace AActorTest
{


	class AMockActor : public AActor
	{
		REFLECT_CLASS(AMockActor, AActor)
	public:
	};

	class UMockSceneComponent : public USceneComponent
	{
		REFLECT_CLASS(UMockSceneComponent, USceneComponent)
	public:
	};

	TEST(TestActor, WhenRemoveAddedComponent_ReturnTrue)
	{
		AMockActor* mockActor = FObjectFactory::ConstructObject<AMockActor>();
		EXPECT_EQ(mockActor->GetClass(), AMockActor::GetClass());

		UMockSceneComponent* mockSceneComponent
			= FObjectFactory::ConstructObject<UMockSceneComponent>(
				FVector(0.0f, 0.0f, 0.0f),
				FRotator(0.0f, 0.0f, 0.0f),
				FVector(1.0f, 1.0f, 1.0f)
			);

		uint32 uuid = mockSceneComponent->UUID;
		mockActor->AddComponent(mockSceneComponent);
		EXPECT_TRUE(mockActor->RemoveComponent(uuid));

		delete mockActor;
		delete mockSceneComponent;
	}

	TEST(TestActor, WhenRemoveNotAddedComponent_ReturnFalse)
	{
		AMockActor* mockActor = FObjectFactory::ConstructObject<AMockActor>();
		EXPECT_EQ(mockActor->GetClass(), AMockActor::GetClass());

		UMockSceneComponent* mockSceneComponent
			= FObjectFactory::ConstructObject<UMockSceneComponent>(
				FVector(0.0f, 0.0f, 0.0f),
				FRotator(0.0f, 0.0f, 0.0f),
				FVector(1.0f, 1.0f, 1.0f)
			);

		uint32 uuid = mockSceneComponent->UUID;
		EXPECT_FALSE(mockActor->RemoveComponent(uuid));

		delete mockActor;
		delete mockSceneComponent;
	}

	TEST(TestActor, WhenRemoveRemovedComponent_ReturnFalse)
	{
		AMockActor* mockActor = FObjectFactory::ConstructObject<AMockActor>();
		EXPECT_EQ(mockActor->GetClass(), AMockActor::GetClass());

		UMockSceneComponent* mockSceneComponent
			= FObjectFactory::ConstructObject<UMockSceneComponent>(
				FVector(0.0f, 0.0f, 0.0f),
				FRotator(0.0f, 0.0f, 0.0f),
				FVector(1.0f, 1.0f, 1.0f)
			);

		uint32 uuid = mockSceneComponent->UUID;
		mockActor->AddComponent(mockSceneComponent);
		EXPECT_TRUE(mockActor->RemoveComponent(uuid));
		EXPECT_FALSE(mockActor->RemoveComponent(uuid));

		delete mockActor;
		delete mockSceneComponent;
	}
}
