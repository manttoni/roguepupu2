#pragma once

#include <cmath>
#include "Vec2.hpp"

namespace Math
{
	template <typename T> static inline T clamp(const T& value, const T& min, const T& max)
	{
		return std::min(max, std::max(min, value));
	}
	inline double map(double x, double a, double b, double c, double d)
	{
		return c + (x - a) * (d - c) / (b - a);
	}

	// increment safely
	template <typename T> void increment(T& value, const T& max_limit)
	{
		if (value > max_limit - T{1})
		{
			value = max_limit;
			return;
		}
		value = value + T{1};
	}
	template <typename T> void decrement(T& value, const T& min_limit)
	{
		if (value < min_limit + T{1})
		{
			value = min_limit;
			return;
		}
		value = value - T{1};
	}

	inline Vec2 polar_to_cartesian(const Vec2 center, const double radius, const double angle)
	{
		const size_t y = static_cast<size_t>(std::round(center.y - radius * std::sin(angle)));
		const size_t x = static_cast<size_t>(std::round(center.x + radius * std::cos(angle)));
		return Vec2(y, x);
	}
}

