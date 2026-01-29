#pragma once

#include <random>
#include "external/PerlinNoise.hpp"

namespace Random
{
	inline std::mt19937& rng()
	{
		static std::random_device rd;
		static std::mt19937 gen(rd());
		return gen;
	}

	// INT
	inline int randint(const int min, const int max, const size_t seed)
	{
		std::mt19937 gen(seed);
		std::uniform_int_distribution<int> dist(min, max);
		return dist(gen);
	}
	inline int randint(const int min, const int max)
	{
		std::uniform_int_distribution<int> dist(min, max);
		return dist(rng());
	}

	// SIZE_T
	inline size_t randsize_t(const size_t min, const size_t max, std::mt19937& rng_obj = rng())
	{
		std::uniform_int_distribution<size_t> dist(min, max);
		return dist(rng_obj);
	}
	inline size_t randsize_t(const size_t min, const size_t max, const size_t seed)
	{
		std::mt19937 gen(seed);
		std::uniform_int_distribution<size_t> dist(min, max);
		return dist(gen);
	}

	// DOUBLE
	inline double randreal(const double min, const double max)
	{
		std::uniform_real_distribution<> dist(min, max);
		return dist(rng());
	}
	inline double randreal(const double min, const double max, const size_t seed)
	{
		std::mt19937 gen(seed);
		std::uniform_real_distribution<> dist(min, max);
		return dist(gen);
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
}

