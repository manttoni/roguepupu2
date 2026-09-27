#include "game/world/Generator.hpp"

namespace Game::World
{
	Generator::Generator(std::string seed) :
		elevation_noise(seed, elevation_settings)
	{}

	Cell Generator::generate_cell(const GlobalPosition& position) const
	{
		using Terrain = Game::Enum::Terrain;

		const auto elevation = elevation_noise.noise2(position);
		if (elevation > sea_level)
			return Cell(Terrain::Ground);
		else
			return Cell(Terrain::Water);
	}
}
