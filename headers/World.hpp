#pragma once

#include <vector>
#include <deque>
#include <ncurses.h>
#include "PerlinNoise.hpp"
#include "Cell.hpp"
#include "Cave.hpp"

class World
{
	private:
		std::deque<Cave> caves;
	public:
		void generate_cave(const size_t level);
		Cave& get_cave(const size_t level);

	private:
		size_t height;
		size_t width;
	public:
		size_t get_height() const { return height; }
		size_t get_width() const { return width; }

	private:
		entt::registry registry;
	public:
		entt::registry& get_registry() { return registry; }

	private:
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
		std::vector<size_t> find_water_path();
		void set_source_sink();
		void spawn_fungi();
		void spawn_chests();
		void set_rock_colors();

	public:
		World();
		World(	const size_t height,
				const size_t width,
				const double frequency,
				const int seed,
				const int margin_percent,
				const int octaves,
				const double A,
				const double B,
				const double C,
				const double fungus_spawn_chance);

		World(const World& other) = delete;
		World& operator=(const World&& other) = delete;
		World(World&& other) = default;
		World& operator=(World&& other) = default;
};

