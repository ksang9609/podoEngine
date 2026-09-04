#pragma once

#include <type_traits>
#include <cmath>

constexpr float PI = 3.1415926535897932f;

namespace FMath
{
	template<typename T>
	void sincos(T& sin, T& cos, T value)
	{
		sin = std::sin(sin);
		cos = std::cos(cos);
	}
}
