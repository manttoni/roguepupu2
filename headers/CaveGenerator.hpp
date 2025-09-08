#pragma once

#include <vector>
#include <ncurses.h>
#include "PerlinNoise.hpp"
#include "Cell.hpp"
#include "Cave.hpp"

class CaveGenerator
{
	private:
		static constexpr size_t EDGE_WEIGH_MULT = 3;
		static constexpr size_t MAX_ROCK_DENSITY = 9; // max density of rock
		static constexpr double ROCK_FORMATION_CHANCE = 0.1; // chance to leave formations when eroding
		static constexpr bool   STRONG_EROSION = true; // also erodes rock while flowing

		size_t height;
		size_t width;
		size_t size;
		double frequency;
		int seed;
		int octaves;
		int margin;
		std::vector<Cave> caves;

		// form rock using perlin noise as density
		std::vector<Cell> form_rock(const size_t level);

		// form a cave using "water simulation"
		// It calculates the path of least resistance and widens it
		std::vector<Cell> find_water_path(Cave& cave);

	public:
		CaveGenerator();
		CaveGenerator(const size_t height, const size_t width, const double frequency, const int seed, const int margin_percent, const int octaves);
		CaveGenerator(const CaveGenerator& other);
		CaveGenerator operator=(const CaveGenerator& other);

		void generate_cave(const size_t level);
		Cave& get_cave(const size_t level);

};

