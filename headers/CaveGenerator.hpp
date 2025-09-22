#pragma once

#include <vector>
#include <ncurses.h>
#include "PerlinNoise.hpp"
#include "Cell.hpp"
#include "Cave.hpp"

class CaveGenerator
{
	private:
		static constexpr size_t EDGE_WEIGH_MULT = 1;
		static constexpr size_t MAX_ROCK_DENSITY = 9; // max density of rock

		Cave canvas;
		std::vector<Cave> caves;

		size_t height;
		size_t width;
		size_t size;

		double frequency;
		int seed;
		int octaves;
		int margin;
		std::mt19937 rng;
		double erosion_a;	// This makes branches
		double erosion_b;	// Smooth cave
		double erosion_c;	// Smooth cave
		double fungus_spawn_chance;
		void form_rock();
		void form_tunnels();
		void set_source_sink();
		void spawn_fungi();
		void set_rock_colors();

		std::vector<size_t> find_water_path();

	public:
		CaveGenerator();
		CaveGenerator(	const size_t height,
						const size_t width,
						const double frequency,
						const int seed,
						const int margin_percent,
						const int octaves,
						const double A,
						const double B,
						const double C,
						const double fungus_spawn_chance);
		CaveGenerator(CaveGenerator&& other);
		CaveGenerator& operator=(CaveGenerator&& other);

		void generate_cave(const size_t level);
		Cave& get_cave(const size_t level);

};

