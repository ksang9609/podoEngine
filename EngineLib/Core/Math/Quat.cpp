#include "Quat.h"

#include "Rotator.h"

// Convert this quaternion to a rotator.
// roll		= atan2(2 * (wx + yz), 1 - 2 * (x^2 + y^2))
// pitch	= asin(2 * (wy - zx))
// yaw		= atan2(2 * (wz + xy), 1 - 2 * (y^2 + z^2))
FRotator FQuat::Rotator() const
{
	const FQuat q = GetNormalized();

	const float sinPitch = FMath::Clamp(
		2.0f * (q.w * q.y - q.z * q.x),
		-1.0f,
		1.0f);

	if (FMath::Abs(sinPitch) >= 1.0f - 1.e-6f)
	{
		// 수직 방향에서는 yaw와 roll을 독립적으로 결정할 수 없다.
		// 동일한 회전을 roll = 0인 표현으로 반환한다.
		const float pitch = sinPitch > 0.0f ? 90.0f : -90.0f;
		const float yaw =
			FMath::RadiansToDegrees(2.0f * std::atan2(q.z, q.w));

		return FRotator(-pitch, yaw, 0.0f);
	}

	const float roll = std::atan2(
		2.0f * (q.w * q.x + q.y * q.z),
		1.0f - 2.0f * (q.x * q.x + q.y * q.y));

	const float pitch = std::asin(sinPitch);

	const float yaw = std::atan2(
		2.0f * (q.w * q.z + q.x * q.y),
		1.0f - 2.0f * (q.y * q.y + q.z * q.z));

	return FRotator(
		-FMath::RadiansToDegrees(pitch),
		FMath::RadiansToDegrees(yaw),
		-FMath::RadiansToDegrees(roll));
}

FQuat FQuat::GetNormalized() const
{
	const float lenghthSquared = x * x + y * y + z * z + w * w;

	if (lenghthSquared <= SMALL_NUMBER)
	{
		return FQuat::Identity();
	}

	const float inverseLength = 1.0f / FMath::Sqrt(lenghthSquared);

	return FQuat(
		x * inverseLength,
		y * inverseLength,
		z * inverseLength,
		w * inverseLength
	);
}

FQuat Slerp(const FQuat& start, const FQuat& end, float alpha)
{
	alpha = FMath::Clamp(alpha, 0.0f, 1.0f);

	const FQuat a = start.GetNormalized();
	FQuat b = end.GetNormalized();

	float dot = a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;

	if (dot < 0.0f)
	{
		b = FQuat(-b.x, -b.y, -b.z, -b.w);
		dot = -dot;
	}

	dot = FMath::Clamp(dot, 0.0f, 1.0f);

	// If the quaternions are very close, use linear interpolation to avoid division by zero
	if (dot > 0.9995f)
	{
		FQuat result = FQuat(
			a.x + alpha * (b.x - a.x),
			a.y + alpha * (b.y - a.y),
			a.z + alpha * (b.z - a.z),
			a.w + alpha * (b.w - a.w)
		);
		return result.GetNormalized();
	}

	const float theta = std::acos(dot);
	const float inverseSinTheta = 1.0f / std::sin(theta);

	const float startWeight = std::sin((1.0f - alpha) * theta) * inverseSinTheta;
	const float endWeight = std::sin(alpha * theta) * inverseSinTheta;

	return FQuat(
		startWeight * a.x + endWeight * b.x,
		startWeight * a.y + endWeight * b.y,
		startWeight * a.z + endWeight * b.z,
		startWeight * a.w + endWeight * b.w
	).GetNormalized();
}
