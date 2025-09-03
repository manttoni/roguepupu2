#pragma once

#include <vector>
#include "PerlinNoise.hpp"
#include "Cell.hpp"
#include "Stream.hpp"
#include "Cave.hpp"

class CaveGenerator
{
	private:
		/* CAVE SIZE */
		size_t size = 60 * 60; // generates caves of this size
		size_t margin = 5; // try to keep a margin around edges or use max_rock_thickness instead

		/* ROCK */
		double frequency = 0.1 // change smoothness of rock thickness randomization
		double rock_formation_chance = 0.1; // chance to leave formations when eroding
		int octaves = 8; // some value for random generator (Perlin)
		int max_rock_thickness = 5; // thicker walls result in bigger caves
		std::vector<Cell> form_rock();

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

	public:
		CaveGenerator();
		Cave generate_cave(); // generate first level
		Cave generate_cave(const Cave& cave); // generate next level

}
