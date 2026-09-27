#include "utils/Perlin.hpp"

namespace Random::Perlin
{
	Generator::Generator(
			std::string_view seed,
			Settings settings) :
		perlin(seed_from_string(seed)),
		settings(settings)
	{
	}

	double Generator::noise2(const Vec2<int>& position) const
	{
		return perlin.normalizedOctave2D_01(
				position.y * settings.frequency,
				position.x * settings.frequency,
				settings.octaves,
				settings.persistence
				);
	}

	siv::PerlinNoise::seed_type seed_from_string(std::string_view text)
	{
		// FNV-1a: a deterministic 32-bit hash.
		std::uint32_t hash = 2166136261u;

		for (const unsigned char character : text)
		{
			hash ^= character;
			hash *= 16777619u;
		}

		return static_cast<siv::PerlinNoise::seed_type>(hash);
	}

}
