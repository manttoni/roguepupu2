#pragma once

#include "Cave.hpp"
#include "entt.hpp"

class CaveGenerator
{
	private:
		struct GenerationConf
		{
			// Size and margin
			size_t size = 150;
			size_t margin = 10;

			// Perlin noise
			double frequency = 0.1;
			size_t octaves = 8;

			// Erosion simulation
			double erosion_a = 2.0;
			double erosion_b = 0.005;
			double erosion_c = 0.005;

			// Water
			size_t max_sources = 1;
			size_t max_sinks = 1;
			double average_water = 0.5;

			// Smoothing
			double smooth_intensity = 0.0;
			size_t smooth_iterations = 0;
		};
		GenerationConf conf;

		// Give each cell a density from Perlin Noise
		// Density range ]0,9] means it is solid rock
		void set_rock_densities(Cave& cave) const;

		// Give some cells water features, source or sink
		// Density -inf means a sink/hole in floor, +inf means a source/hole in ceiling
		void set_water_features(Cave& cave) const;

		// Repeat erosion_simulation until all sources have a sink and visa versa
		void form_tunnels(Cave& cave) const;

		// Create a path of least resistance through the rock, while reducing density
		void erosion_simulation(Cave& cave, const size_t start, const size_t end) const;

		// Smooth terrain but dont break sinks
		void smooth_terrain(Cave& cave) const;

		// Normalize terrain elevation (or skip this)
		// Elevation means negative density which results in erosion_simulation
		void normalize_negative_density(Cave& cave) const;

		// Add some initial water
		void add_water(Cave& cave) const;

		// Pass time to let water drain or collect in pools
		void simulate_environment(Cave& cave, const size_t rounds = 1000) const;

		void spawn_entities(const Cave& cave, entt::registry& registry) const;

		// Pass some time with simulate_cave before player comes in

	public:
		// Construct with conf
		CaveGenerator(const GenerationConf& conf);

		// Pass registry to be able to spawn entities
		Cave generate() const;
};
