#pragma once

#include <random>
#include "external/PerlinNoise.hpp"
#include "utils/Range.hpp"

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

	// PERLIN NOISE
	inline double noise3D(double x, double y, double z, double f, int seed, int octave)
	{
		const siv::PerlinNoise::seed_type perlin_seed = seed;
		const siv::PerlinNoise perlin{ perlin_seed };
		return perlin.octave3D_01(x * f, y * f, z * f, octave);
	}
	inline double noise2D(double y, double x, double frequency, size_t octaves, size_t seed)
	{
		const siv::PerlinNoise::seed_type perlin_seed = seed;
		const siv::PerlinNoise perlin{perlin_seed};
		return perlin.octave2D_01(y * frequency, x * frequency, octaves);
	}

	template<typename T> const T& get_random_element(const std::vector<T>& vec)
	{
		assert(!vec.empty());
		const auto index = rand<size_t>(0, vec.size() - 1);
		return vec[index];
	}
}

