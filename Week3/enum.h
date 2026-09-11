#pragma once

#include <stdexcept>

enum class EAxis : int { X = 0, Y = 1, Z = 2 };

enum class EPrimitive
{
	EP_Sphere,
	EP_Cube,
	EP_Triangle,
	EP_GizmoArrow,
	EP_Circle,
};

enum EGIZMO_AXIS //어떤축이 선택되었는지
{
	NONE,
	X,
	Y,
	Z
};

enum EGIZMO_TYPE {
	TRANSLATE,
	ROTATE,
	SCALE,
};

inline EPrimitive StringToEPrimitive(const char* str)
{
	if (strcmp(str, "Sphere") == 0)
	{
		return EPrimitive::EP_Sphere;
	}
	else if (strcmp(str, "Cube") == 0)
	{
		return EPrimitive::EP_Cube;
	}
	else if (strcmp(str, "Triangle") == 0)
	{
		return EPrimitive::EP_Triangle;
	}
	else if (strcmp(str, "GizmoArrow") == 0)
	{
		return EPrimitive::EP_GizmoArrow;
	}
	else if (strcmp(str, "Circle") == 0)
	{
		return EPrimitive::EP_Circle;
	}
	else
	{
		throw std::runtime_error("Unknown EPrimitive value");
	}
}

inline const char* PrimitiveToString(EPrimitive primitiveType)
{
	switch (primitiveType)
	{
	case EPrimitive::EP_Cube:
		return "Cube";

	case EPrimitive::EP_Sphere:
		return "Sphere";

	case EPrimitive::EP_Triangle:
		return "Triangle";

	case EPrimitive::EP_GizmoArrow:
		return "GizmoArrow";

	case EPrimitive::EP_Circle:
		return "Circle";

	default:
		return "Unknown";
	}
}
