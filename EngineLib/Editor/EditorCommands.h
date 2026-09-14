#include "Core/Core.h"
#include "Core/enum.h"

/* Editor Commands */
/* SceneManager Commands */
struct FNewSceneCommand {};
struct FSaveSceneCommand { FString SceneName; };
struct FLoadSceneCommand { FString SceneName; };

struct FSpawnActorCommand { EPrimitive PrimitiveType; int32 SpawnCount; };
struct FDeleteActorCommand { FObjectID ObjectID; };

struct FSetActorLocationCommand { FObjectID ObjectID; FVector Location; };
struct FSetActorRotationCommand { FObjectID ObjectID; FRotator Rotation; };
struct FSetActorScaleCommand { FObjectID ObjectID; FVector Scale; };
struct FSetActorNameCommand { FObjectID ObjectID; FName NewName; };
struct FSetSelectedActorCommand { FObjectID ObjectID; };

/* EditorViewportClient Commands */
struct FSetViewModeCommand { EViewModeIndex ViewMode; };
struct FSetShowFlagCommand { uint32 ShowFlags; };
struct FSetCameraSensitivityCommand { float Sensitivity; };
struct FSetCameraFovCommand { float Fov; };
struct FSetCameraLocationCommand { FVector Location; };
struct FSetCameraRotationCommand { FRotator Rotation; };
struct FSetGizmoModeCommand { EGIZMO_TYPE GizmoMode; };
struct FCycleGizmoModeCommand {};

/* GraphicsManager Commands */
struct FSetGridWidthCommand { float GridWidth; };
struct FStartProjectionTransitionCommand { bool bOrthographic; };

using FEditorCommand = std::variant <
	FNewSceneCommand,
	FSaveSceneCommand,
	FLoadSceneCommand,
	FSpawnActorCommand,
	FDeleteActorCommand,
	FSetActorLocationCommand,
	FSetActorRotationCommand,
	FSetActorScaleCommand,
	FSetActorNameCommand,
	FSetSelectedActorCommand,
	FSetViewModeCommand,
	FSetShowFlagCommand,
	FSetCameraSensitivityCommand,
	FSetCameraFovCommand,
	FSetCameraLocationCommand,
	FSetCameraRotationCommand,
	FSetGizmoModeCommand,
	FCycleGizmoModeCommand,
	FSetGridWidthCommand,
	FStartProjectionTransitionCommand
> ;
using FEditorCommands = TArray<FEditorCommand>;
