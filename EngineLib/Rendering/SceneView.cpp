#include "SceneView.h"

#include "Camera.h"
#include "../Core/Math/MathUtility.h"

FSceneView makeSceneView(const FCamera& camera, const FViewRect& rect, float projectionratio, float nearz, float farz)
{
	FSceneView view;
	view.Rect = rect;
	if (!rect.isValid())
	{
		return view;
	}
	view.cameraLocation = camera.Location;
	view.cameraRotation = camera.Rotation;
	view.cameraForward = camera.GetForwardVector();
	view.cameraRight = camera.GetRightVector();
	view.cameraUp = camera.GetUpVector();
	view.fovDegree = camera.mFovDegree;
	view.orthoDistance = camera.mOrthoDistance;
	view.projectionRatio = FMath::Clamp(projectionratio, 0.0f, 1.0f);
	view.nearZ = nearz;
	view.farZ = farz;
	const float aspect = rect.getAspectRatio();
	view.viewMatrix = camera.GetViewMatrix();
	view.projectionMatrix = camera.GetUnifiedProjectionMatrix(aspect, view.fovDegree, view.orthoDistance, view.nearZ, view.farZ, view.projectionRatio);
	view.viewProjectionMatrix = view.viewMatrix * view.projectionMatrix;
	view.inverseViewprojectionMatrix = view.viewProjectionMatrix.Inverse();
	return view;
}
