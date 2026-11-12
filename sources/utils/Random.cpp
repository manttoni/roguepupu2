#include <random>
#include <stdexcept>
#include "utils/Random.hpp"

namespace
{
	std::uint64_t mix(std::uint64_t value)
	{
		value += 0x9e3779b97f4a7c15ULL;
		value = (value ^ (value >> 30)) * 0xbf58476d1ce4e5b9ULL;
		value = (value ^ (value >> 27)) * 0x94d049bb133111ebULL;
		return value ^ (value >> 31);
	}
}

namespace Random
{
	std::uint64_t hash_seed(const std::string_view seed)
	{
		std::uint64_t result = 14695981039346656037ULL;
		for (const unsigned char character : seed)
		{
			result ^= character;
			result *= 1099511628211ULL;
		}
		return result;
	}

	double at(
		const std::uint64_t seed,
		const Game::World::GlobalPosition& position,
		const std::uint64_t stream)
	{
		auto value = mix(seed ^ stream);
		value = mix(value ^ static_cast<std::uint64_t>(position.y));
		value = mix(value ^ static_cast<std::uint64_t>(position.x));

		// Convert the upper 53 bits to a double in [0, 1).
		return static_cast<double>(value >> 11) * 0x1.0p-53;
	}

	namespace
	{
		std::mt19937& engine()
		{
			static std::mt19937 generator{std::random_device{}()};
			return generator;
		}
	}

	void seed(const std::uint32_t value)
	{
		engine().seed(value);
	}

	// Both bounds are inclusive.
	int integer(const int min, const int max)
	{
		if (min > max)
			throw std::invalid_argument("Random minimum exceeds maximum");

		std::uniform_int_distribution<int> distribution{min, max};
		return distribution(engine());
	}
}
