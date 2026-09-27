#pragma once

#include <random>
#include "external/PerlinNoise.hpp"
#include "utils/Range.hpp"
#include "utils/Vec2.hpp"

namespace Random
{
	inline std::mt19937& rng()
	{
		static std::random_device rd;
		static std::mt19937 gen(rd());
		return gen;
	}

	template<typename T>
		inline T rand(const T min, const T max, std::mt19937& engine = rng())
		{
			static_assert(std::is_arithmetic_v<T>, "T must be numeric");
			if constexpr (std::is_integral_v<T>)
			{
				std::uniform_int_distribution<T> dist(min, max);
				return dist(engine);
			}
			else
			{
				std::uniform_real_distribution<T> dist(min, max);
				return dist(engine);
			}
		}

	template<typename T>
		inline T rand(const Range<T> range, std::mt19937& engine = rng())
		{
			return rand(range.min, range.max, engine);
		}

	inline bool roll(const double chance)
	{
		return chance >= rand<double>(0.0, 1.0);
	}
}

