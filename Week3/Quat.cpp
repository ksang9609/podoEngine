#include "Quat.h"

#include "Rotator.h"

FRotator FQuat::Rotator() const
{
	float roll = std::atan2(2.0f * (w * x + y * z), 1.0f - 2.0f * (x * x + y * y));
	float pitch = std::asin(2.0f * (w * y - z * x));
	float yaw = std::atan2(2.0f * (w * z + x * y), 1.0f - 2.0f * (y * y + z * z));
	return FRotator(-pitch * 180.0f / PI, yaw * 180.0f / PI, -roll * 180.0f / PI);
}
