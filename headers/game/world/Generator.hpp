#pragma once

#include "game/world/Cell.hpp"
#include <string>
#include "game/world/Position.hpp"
#include "utils/Perlin.hpp"

namespace Game::World
{
	class Generator
	{
		public:
			Generator(std::string seed);
			Cell generate_cell(const GlobalPosition& position) const;

		private:
			Random::Perlin::Generator elevation_noise;
			static constexpr double sea_level = 0.5;
			static constexpr Random::Perlin::Settings elevation_settings{};
	};
}
