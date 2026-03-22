#pragma once

#include "external/entt/entt.hpp"
#include "domain/Cave.hpp"
#include "utils/Parser.hpp"

namespace CaveGenerator
{
	struct Data
	{
		struct Density
		{
			double frequency;
			size_t octaves;
		};

		struct Erosion
		{
			double primary;
			double secondary;
		};

		struct Smooth
		{
			double intensity;
			size_t iterations;
			bool rock;
		};

		struct Features
		{
			size_t sinks;
			size_t sources;
		};

		struct Margin
		{
			size_t size;
			size_t multiplier;
		};

		entt::registry& registry;
		Cave& cave;

		Density density;
		Erosion erosion;
		Smooth smooth;
		Features features;
		Margin margin;

		Data(entt::registry& registry, Cave& cave)
			: registry(registry), cave(cave)
		{
			auto conf = Parser::read_json_file("data/generation/cave.json");

			density.frequency = conf["density"]["frequency"].get<double>();
			density.octaves   = conf["density"]["octaves"].get<size_t>();

			erosion.primary   = conf["erosion"]["primary"].get<double>();
			erosion.secondary = conf["erosion"]["secondary"].get<double>();

			smooth.intensity  = conf["smooth"]["intensity"].get<double>();
			smooth.iterations = conf["smooth"]["iterations"].get<size_t>();
			smooth.rock       = conf["smooth"]["rock"].get<bool>();

			features.sinks   = conf["features"]["sinks"].get<size_t>();
			features.sources = conf["features"]["sources"].get<size_t>();

			margin.size       = conf["margin"]["size"].get<size_t>();
			margin.multiplier = conf["margin"]["multiplier"].get<size_t>();
		}
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
	void erosion_simulation(Data& data, const Position& start, const Position& end);

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

	// Generate a cave in a blank Cave object
	// Has to be indexed in World.
	// prompt is temporarily for inspecting result
	void generate(entt::registry& registry, const size_t cave_idx, const bool prompt = false);
};
