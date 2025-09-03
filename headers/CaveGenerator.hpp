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
		/* CAVE SIZE */
		size_t height;
		size_t width;
		size_t size; // generates caves of this size
		static constexpr size_t MARGIN_PERCENT = 15;
		size_t margin; // if distance to edges less than this increase ROCK_DENSITY
		static constexpr size_t EDGE_WEIGH_MULT = 3;

		/* ROCK */
		static constexpr double PERLIN_FREQUENCY = 0.1; // change smoothness of rock thickness randomization
		static constexpr size_t PERLIN_OCTAVES = 8; // some value for random generator (Perlin)
		static constexpr size_t ROCK_DENSITY = 9; // max density of rock
		double rock_formation_chance = 0.1; // chance to leave formations when eroding
		std::vector<Cell> form_rock(const size_t level);

		/* WATER */
		double min_stream_length = 10; // distance from any source to any sink
		double min_source_distance = 10; // distance from sources to other sources
		double min_sink_distance = 10; // distance from sinks to other sinks
		int source_count = 1;
		int sink_count = 2;
		std::vector<size_t> stream_sources, stream_sinks;
		std::vector<Cell> stream_paths;
		bool stronger_erosion = true; // also erodes rock while flowing
		Stream generate_stream(); // generates a stream
		void flow_water(const size_t start_id, const size_t end_id);
		void form_cave();

		/* CELLS */
		void erode_cell(Cell& cell);

		/* CAVE NETWORK */
		std::vector<Cave> caves;

		/* DEBUG */
		void print_cells(const std::vector<Cell>& cells);

		/* GENERATOR ATTRIBUTES */
		size_t seed;

	public:
		CaveGenerator();
		void generate_cave(const size_t level);
		Cave get_cave(const size_t level);

};

