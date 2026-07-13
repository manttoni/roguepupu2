#include <math.h>
#include <stddef.h>
#include <nlohmann/detail/json_ref.hpp>
#include <nlohmann/json.hpp>
#include <algorithm>
#include <cassert>
#include <vector>
#include <limits>
#include <iterator>
#include <map>
#include <string>
#include <utility>

#include "infrastructure/EventQueue.hpp"
#include "UI/Dialog.hpp"
#include "UI/UI.hpp"
#include "domain/Cave.hpp"
#include "domain/Cell.hpp"
#include "domain/Liquid.hpp"
#include "domain/LiquidMixture.hpp"
#include "domain/Position.hpp"
#include "generation/CaveGenerator.hpp"
#include "generation/EntitySpawner.hpp"
#include "infrastructure/GameState.hpp"
#include "systems/environment/LiquidSystem.hpp"
#include "systems/position/MovementSystem.hpp"
#include "systems/rendering/LightingSystem.hpp"
#include "systems/rendering/RenderingSystem.hpp"
#include "utils/Log.hpp"
#include "utils/Math.hpp"
#include "utils/Random.hpp"
#include "utils/Vec2.hpp"
#include "utils/ECS.hpp"

namespace CaveGenerator
{
	// Make map edges unpassable
	bool is_on_edge(const Data& data, const Vec2<int>& coords)
	{
		const auto margin_size = static_cast<int>(data.margin.size);
		const auto cave_size = static_cast<int>(data.cave.get_size());
		return
			coords.y < margin_size ||
			coords.x < margin_size ||
			coords.y > cave_size - margin_size - 1 ||
			coords.x > cave_size - margin_size - 1;
	}

	/* Give each cell a density [1,9] from Perlin Noise
	 * Make edges denser to keep erosion_simulation away from them
	 * */
	void set_rock_densities(Data& data)
	{
		Log::info() << "Setting rock densities";
		auto& cells = data.cave.get_cells();
		const size_t seed = Random::rand<size_t>(0, 999999);
		for (auto& cell : cells)
		{
			const Vec2<int> coords = Vec2<int>::from_idx(cell.get_idx(), data.cave.get_size());
			const double perlin = Random::noise2D(
					coords.y,
					coords.x,
					data.density.frequency,
					data.density.octaves,
					seed);
			const double density = Math::map(perlin, 0, 1, 1, CELL_DENSITY_MAX);
			cell.set_density(density * (
						is_on_edge(data, coords) ?
						data.margin.multiplier :
						1
						));
		}
	}

	/* Set -inf for sinks because LiquidSystem will then flow any amount into it
	 * Set +inf for sources
	 * Divide cave like a pizza, each slice gets one feature.
	 * This is a bit crazy, if it breaks just make a simpler one.
	 * */
	void set_erosion_points(Data& data)
	{
		const size_t total_features = data.features.sources + data.features.sinks;
		std::map<Cell::Type, size_t> spawning_features =
		{
			{ Cell::Type::Source, data.features.sources },
			{ Cell::Type::Sink, data.features.sinks }
		};
		const Vec2<int> center(static_cast<int>(data.cave.get_size()) / 2, static_cast<int>(data.cave.get_size()) / 2);
		const double max_radius =
			static_cast<double>(data.cave.get_size()) / 2 -
			static_cast<double>(data.margin.size);
		for (size_t i = 0; i < total_features; ++i)
		{
			const double angle_min = 2.0 * M_PI *
				static_cast<double>(i) /
				static_cast<double>(total_features);
			const double angle_max = 2.0 * M_PI *
				static_cast<double>(i + 1) /
				static_cast<double>(total_features);
			const double angle = Random::rand<double>(angle_min, angle_max);
			const double radius = Random::rand<double>(max_radius / 1.5, max_radius);

			const Vec2<int> coords = Math::polar_to_cartesian(center, radius, angle);
			const size_t cell_idx = coords.to_idx(data.cave.get_size());
			const Position pos(cell_idx, data.cave.get_idx());
			auto& cell = data.cave.get_cell(pos);
			if (cell.get_type() != Cell::Type::Rock)
			{
				i--; // Try again to find suitable cell. Probably won't reach this.
				continue;
			}

			// Decide which feature to spawn here
			const size_t options = spawning_features.size();
			assert(options > 0);
			const size_t rand = Random::rand<size_t>(0, options - 1);
			auto it = std::next(spawning_features.begin(), rand);
			cell.set_type(it->first);
			it->second--;
			if (it->second == 0)
				spawning_features.erase(it);
		}

		// If this is the first level, or idx == 0,
		// player character spawn point is in the middle
		if (data.cave.get_idx() == 0)
		{
			auto middle_pos = data.cave.middle_position();
			data.cave.get_cell(middle_pos).set_type(Cell::Type::Source); // this position will now be a part of the tunnels
		}
		Log::info() << "Set erosion points: " << total_features;

		assert(spawning_features.empty());
	}

	// This is for inspecting the result during generation
	void render(Data& data)
	{
		if (data.registry.ctx().get<GameState>().test_run)
			return;
		LightingSystem::reset_lights(data.registry, data.cave.get_idx());
		RenderingSystem::render_generation(data.registry, data.cave.get_idx());
	}

	void simulate_environment(Data& data)
	{
		for (size_t i = 0; i < 32; ++i)
			LiquidSystem::simulate_liquids(data.registry, data.cave.get_idx());
	}

	void form_tunnels(Data& data)
	{
		const bool test_run = data.registry.ctx().get<GameState>().test_run;
		const auto& sources = data.cave.get_positions_with_type(Cell::Type::Source);
		const auto& sinks = data.cave.get_positions_with_type(Cell::Type::Sink);

		bool flag = false;
		std::vector<std::pair<Position, Position>> connections;
		const auto max_connections = sources.size() * sinks.size();
		while (flag == false)
		{
			render(data);
			if (!test_run)
				Dialog::message(std::to_string(connections.size()) + "/" + std::to_string(max_connections));
			flag = true; // If this will not get set to false, then everything is connected
			for (const auto& source : sources)
			{
				for (const auto& sink : sinks)
				{
					// The coordinates are connected with a tunnel, if pathfinder found it and it was recorded into 'connections'
					const auto is_connected = std::find(connections.begin(), connections.end(), std::pair<Position, Position>(source, sink)) != connections.end();
					if (is_connected)
						continue;

					// Try to connect, erode rock during it
					if (erosion_simulation(data, source, sink) == 0) // 0 obstacles left
					{
						connections.push_back({source, sink});
						continue;
					}

					// smooth
					// smooth_terrain(data);

					// Let water flow naturally
					// A more efficient alternative would be to set a water level height once in the end
					simulate_environment(data);

					flag = false; // not ready yet
				}
			}
		}

		// Disable sources and sinks
		for (const auto& source : sources)
			ECS::get_cell(data.registry, source).set_type(Cell::Type::Floor);
		for (const auto& sink : sinks)
			ECS::get_cell(data.registry, sink).set_type(Cell::Type::Floor);

		Log::info() << "Form tunnels. Connections: " << connections.size() << "/" << max_connections;
	}

	// A* to find path of least resistance through solid rock
	// return 0 if ready
	size_t erosion_simulation(Data& data, const Position& start, const Position& end)
	{
		auto& cave = data.cave;

		std::vector<Position> open_set = { start };
		std::map<Position, double> g_score;
		std::map<Position, double> f_score;
		g_score[start] = 0;
		f_score[start] = cave.distance(start, end);
		size_t obstacles = 0; // count how many times have to pass solid rock
		while (!open_set.empty())
		{
			auto current_pos = open_set[0];
			for (const auto cell_pos : open_set)
			{	// all open_set elements have f_score mapped
				if (f_score[cell_pos] < f_score[current_pos])
					current_pos = cell_pos;
			}

			assert(current_pos.is_valid() && "Position is not valid");

			const auto current_density = cave.get_cell(current_pos).get_density();

			if (current_density > 0 && current_density <= CELL_DENSITY_MAX)
			{
				// Render blockages with more red
				auto& color = cave.get_cell(current_pos).get_bgcolor();
				color += Color(1, 0, 0);
				//Log::debug() << color;
				obstacles++;
			}
			if (current_pos == end)
				return obstacles;

			cave.get_cell(current_pos).reduce_density(data.erosion.primary);

			open_set.erase(std::find(open_set.begin(), open_set.end(), current_pos)); // is found
			for (const auto neighbor_pos : cave.get_nearby_positions(current_pos, 1.5))
			{
				// Clamp density, otherwise goes crazy
				const double density = std::max(0.0, cave.get_cell(neighbor_pos).get_density());
				double tentative_g_score = g_score[current_pos] + density * density;

				// if neighbor doesnt have a g_score, init it to inf
				if (g_score.count(neighbor_pos) == 0)
					g_score[neighbor_pos] = std::numeric_limits<double>::infinity();

				cave.get_cell(neighbor_pos).reduce_density(data.erosion.secondary);

				if (tentative_g_score < g_score[neighbor_pos])
				{
					g_score[neighbor_pos] = tentative_g_score;
					f_score[neighbor_pos] = tentative_g_score +
						cave.distance(neighbor_pos, end) *
						Random::rand<double>(0.9,1.0);
					auto it = std::find(open_set.begin(), open_set.end(), neighbor_pos);
					if (it == open_set.end())
						open_set.push_back(neighbor_pos);
				}
			}
		}
		return obstacles;
	}


	void generate(entt::registry& registry, const size_t cave_idx)
	{
		Log::info() << "Generating cave " << cave_idx;
		auto& cave = ECS::get_cave(registry, cave_idx); // Cave must exist in the World object
		const bool testing = registry.ctx().get<GameState>().test_run;
		if (!testing)
			UI::instance().set_current_panel(UI::Panel::Game, true); // true = render panel on top

		Data data(registry, cave); // Parses conf from data/generation/cave.json

		/* Put cave forming stuff here, "geology"
		 * */
		set_rock_densities(data); // Give each cell a value [1, CELL_MAX_DENSITY] randomized by Perlin noise
		set_erosion_points(data); // Give some cells an "erosion entity", at least one source and one sink
		form_tunnels(data); // Connect erosion points with a special pathfinding algorithm

		/* Spawn entities
		 * */
		EntitySpawner::spawn_natural_entities(registry, cave_idx);

		/* Rendering the cave while generating or after is preferred when testing manually
		 * RenderingSystem has a function for rendering the cave in a simple way
		 * */
		render(data);

		if (!testing)
			Dialog::alert("Cave ready");
	}
}
