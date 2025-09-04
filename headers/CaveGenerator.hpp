#pragma once

#include <vector>
#include <ncurses.h>
#include "PerlinNoise.hpp"
#include "Cell.hpp"
#include "Stream.hpp"
#include "Cave.hpp"

class CaveGenerator
{
	private:
		static constexpr size_t MARGIN_PERCENT = 15;
		static constexpr size_t EDGE_WEIGH_MULT = 3;
		static constexpr double PERLIN_FREQUENCY = 0.1; // change smoothness of rock thickness randomization
		static constexpr size_t PERLIN_OCTAVES = 8; // some value for random generator (Perlin)
		static constexpr size_t ROCK_DENSITY = 9; // max density of rock
		static constexpr double ROCK_FORMATION_CHANCE = 0.1; // chance to leave formations when eroding
		static constexpr bool   STRONG_EROSION = true; // also erodes rock while flowing
									  //
		size_t height;
		size_t width;
		size_t size; // generates caves of this size
		size_t margin; // if distance to edges less than this increase ROCK_DENSITY
		size_t seed;
		std::vector<Cave> caves;

		std::vector<Cell> form_rock(const size_t level);

	public:
		CaveGenerator(const size_t height, const size_t width);
		void generate_cave(const size_t level);
		Cave get_cave(const size_t level);

};

