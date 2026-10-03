#pragma once
#include <cstdint>
#include <string_view>

#include "external/PerlinNoise.hpp"
#include "utils/Vec2.hpp"

namespace Random::Perlin
{
	struct Settings
	{
		double frequency = 0.01;
		std::int32_t octaves = 4;
		double persistence = 0.5;
	};

	class Generator
	{
		public:
			Generator(std::string_view seed, Settings settings = {});
			double noise2(const Vec2<int>& position) const;

		private:
			siv::PerlinNoise perlin;
			Settings settings;
	};

	siv::PerlinNoise::seed_type seed_from_string(std::string_view text);
}
