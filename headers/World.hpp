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

		double mushroom_spawn_chance;
		double mushroom_frequency;
		size_t mushroom_octaves;
		size_t mushroom_woody_radius;
		double mushroom_woody_space_ratio;

		double chest_spawn_chance;
		size_t chest_value_power;
		size_t chest_value_multiplier;
		int chest_value_scalar;
		size_t chest_item_variance;

		void form_rock();
		void form_tunnels();
		std::vector<size_t> find_water_path();
		void set_source_sink();
		void spawn_mushrooms();
		void spawn_chests();
		void set_rock_colors();

	public:
		World();

		World(const World& other) = delete;
		World& operator=(const World&& other) = delete;
		World(World&& other) = default;
		World& operator=(World&& other) = default;
};

