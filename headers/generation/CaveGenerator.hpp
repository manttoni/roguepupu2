#pragma once

#include "external/entt/entt.hpp"
#include "domain/Cave.hpp"

namespace CaveGenerator
{
	struct Data
	{
		struct Density
		{
			double frequency = 0.1;	// Smaller values produce smoother diff in density
			size_t octaves = 8;		// More irregularities in density
		};
		struct Erosion
		{	// Premake these for different types of caves.
			// These can be set to create smooth lavatubes or jagged karst caves.
			// f.e. lava could have high erosion_a while fresh water has low.
			// Low erosion values will take longer. One way is to use high values with big size,
			// and low values with small size
			double erosion_a = 2.0;		// Erode cell with lowest f_score
			double erosion_b = 0.005;	// Each of that cells neighbors
			double erosion_c = 0.0;		// When finding new best g_score
		};
		struct Smooth
		{
			double intensity = 0.0;
			size_t iterations = 0;
		};
		struct Features
		{	// What geological features does the cave have.
			// Could have some Chambers or something
			//
			// These are the basic way to generate caves
			size_t sinks = 1;	// water flows here
			size_t sources = 1;	// from here
			// Chamber chamber?
		};
		struct Margin
		{
			size_t size = 10;
			size_t multiplier = 2;
		};

		entt::registry& registry;	// To be able to use systems and spawn entities
		Cave& cave;					// This has to be indexed in World
		Density density;			// Set initial densities
		Erosion erosion;			// Change this if want some specific kind of erosion
		Smooth smooth;
		Features features;			// Change this to create more tunnels and more?
		Margin margin;				// Make edges denser

		Data(entt::registry& registry, Cave& cave) : registry(registry), cave(cave) {}
	};

	// Give each cell a density [1,9] from Perlin Noise
	// Density range ]0,9] means it is solid rock
	// Density 0 or smaller means open space
	// The smaller the density the lower the elevation
	void set_rock_densities(Data& data);

	// Give some cells water features, source or sink
	// Density -inf means a sink/hole in floor, +inf means a source/hole in ceiling
	// Determine whether liquid will come out and what type
	// Without at least one of each, the cave cannot be generated
	void set_water_features(Data& data);

	// Repeat erosion_simulation until all sources have a sink and visa versa
	void form_tunnels(Data& data);

	// Create a path of least resistance through the rock, while reducing density
	void erosion_simulation(Data& data, const size_t start, const size_t end);

	// Smooth terrain without changing liquid sinks so it will drain properly/as intended
	void smooth_terrain(Data& data);

	// Add at least 2 passages to each cave.
	// Use entities with Transition component.
	// Transition component will generate next cave when player moves on it.
	// For now just place them above water level
	void add_passages(Data& data);

	// Normalize terrain elevation (or skip this)
	// Elevation means negative density which results in erosion_simulation
	// Probably not good to use this... Better to keep cave elevation as is
	//void normalize_negative_density(Cave& cave) const;

	// Pass time until liquid has equilibrium
	void stabilize_liquids(Data& data);

	// Evaluate all entities with spawn data in json
	void spawn_entities(Data& data);

	// Modify an existing cave in World.
	// Has to be indexed in World.
	void generate_cave(Data& data);
};
