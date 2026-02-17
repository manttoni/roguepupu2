#pragma once

#include <cmath>
#include "Vec2.hpp"

namespace Math
{
	template <typename T> static inline T clamp(const T& value, const T& min, const T& max)
	{
		if constexpr (std::is_arithmetic_v<T>)
			return std::min(max, std::max(min, value));
		else if constexpr (std::is_same_v<T, nlohmann::json>)
		{
			if (value.is_number_integer())
				return nlohmann::json{
					clamp(value.template get<int>(), min.template get<int>(), max.template get<int>())
				};
			else if (value.is_number_float())
				return nlohmann::json{
					clamp(value.template get<double>(), min.template get<double>(), max.template get<double>())
				};
			else
				return value;
		}
		else
			return value;
	}
	inline double map(double x, double a, double b, double c, double d)
	{
		return c + (x - a) * (d - c) / (b - a);
	}

	inline double round1(const double v)
	{
		return std::round(v * 10.0) / 10.0;
	}

	inline size_t get_precision(double d)
	{
		size_t precision = 0;
		while (d < 1.0)
		{
			precision++;
			d *= 10;
		}
		return precision;
	}

	inline Vec2 polar_to_cartesian(const Vec2 center, const double radius, const double angle)
	{
		const int y = static_cast<int>(std::round(center.y - radius * std::sin(angle)));
		const int x = static_cast<int>(std::round(center.x + radius * std::cos(angle)));
		return Vec2(y, x);
	}
}

