#pragma once

#include <random>    // for mt19937
#include "Cave.hpp"  // for Cave
#include "entt.hpp"  // for size_t, registry, deque, vector
#include "nlohmann/json.hpp"

#define DENSITY_MAX 10

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


		std::vector<size_t> get_empty_cells(const Cave& cave);
		void form_rock();
		void form_tunnels();
		std::vector<size_t> find_water_path(const bool clamp_density = true);
		void set_source_sink();
		size_t randomize_transition_idx(const size_t other = 0);
		void set_rock_colors();
		void set_humidity();
		void spawn_entities(nlohmann::json& filter);

	public:
		World();

		World(const World& other) = delete;
		World& operator=(const World&& other) = delete;
		World(World&& other) = default;
		World& operator=(World&& other) = default;
};

