#include <algorithm>
#include <cassert>
#include <vector>
#include <limits>
#include "UI/UI.hpp"
#include "domain/Cave.hpp"
#include "domain/Cell.hpp"
#include "domain/LiquidMixture.hpp"
#include "domain/Position.hpp"
#include "generation/CaveGenerator.hpp"
#include "systems/environment/LiquidSystem.hpp"
#include "systems/position/MovementSystem.hpp"
#include "systems/rendering/RenderingSystem.hpp"
#include "utils/Math.hpp"
#include "utils/Random.hpp"
#include "generation/EntitySpawner.hpp"
#include "systems/rendering/LightingSystem.hpp"
#include "UI/Dialog.hpp"

namespace CaveGenerator
{
	void echo(const std::string& message)
	{
		Log::log(message);
		Dialog::show_message(message);
	}
	bool is_on_edge(const Data& data, const Vec2& coords)
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
		echo("Setting rock densities...");
		auto& cells = data.cave.get_cells();
		const size_t seed = Random::randsize_t(0, 99999);
		for (auto& cell : cells)
		{
			const Vec2 coords(cell.get_idx(), data.cave.get_size());
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
	 * Set +inf for sources for the vibes
	 * Divide cave like a pizza, each slice gets one feature.
	 * */
	void set_water_features(Data& data)
	{
		echo("Setting water features...");
		const size_t total_features = data.features.sources + data.features.sinks;
		std::map<Cell::Type, size_t> spawning_features =
		{
			{ Cell::Type::Source, data.features.sources },
			{ Cell::Type::Sink, data.features.sinks }
		};
		const Vec2 center(static_cast<int>(data.cave.get_size()) / 2, static_cast<int>(data.cave.get_size()) / 2);
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
			const double angle = Random::randreal(angle_min, angle_max);
			const double radius = Random::randreal(max_radius / 1.5, max_radius);

			const Vec2 coords = Math::polar_to_cartesian(center, radius, angle);
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
			const size_t rand = Random::randsize_t(0, options - 1);
			auto it = std::next(spawning_features.begin(), rand);
			cell.set_type(it->first);
			it->second--;
			if (it->second == 0)
				spawning_features.erase(it);

			// Make some sources spawn liquids, testing with 100% chance to spawn 1l per frame
			if (cell.get_type() == Cell::Type::Source)
			{
				const LiquidMixture liquid_source(Liquid::Type::Water, 1);
				cell.set_liquid_source(liquid_source);
			}
		}
		assert(spawning_features.empty());
	}

	void render(Data& data)
	{
		LightingSystem::reset_lights(data.registry, data.cave.get_idx());
		RenderingSystem::render_generation(data.registry, data.cave.get_idx());
	}

	void set_entities(Data& data)
	{
		echo("Setting entities...");
		EntitySpawner::despawn_entities(data.registry, data.cave.get_idx());
		EntitySpawner::spawn_entities(data.registry, data.cave.get_idx(), {{"category", "nature"}});

	}

	void simulate_environment(Data& data)
	{
		for (size_t i = 0; i < 32; ++i)
			LiquidSystem::simulate_liquids(data.registry, data.cave.get_idx());
	}

	void form_tunnels(Data& data)
	{
		echo("Forming tunnels...");
		const auto& sources = data.cave.get_positions_with_type(Cell::Type::Source);
		const auto& sinks = data.cave.get_positions_with_type(Cell::Type::Sink);

		bool flag = false;
		while (flag == false)
		{
			flag = true;
			for (const auto& source : sources)
			{
				for (const auto& sink : sinks)
				{
					erosion_simulation(data, source, sink);
					smooth_terrain(data);
					simulate_environment(data);
					if (MovementSystem::find_path(data.registry, source, sink, false).empty())
					{

						flag = false;
					}
				}
			}
			render(data);
		}
	}

	// A* to find path of least resistance through solid rock
	void erosion_simulation(Data& data, const Position& start, const Position& end)
	{
		auto& cave = data.cave;

		std::vector<Position> open_set = { start };
		std::map<Position, double> g_score;
		std::map<Position, double> f_score;
		g_score[start] = 0;
		f_score[start] = cave.distance(start, end);
		while (!open_set.empty())
		{
			auto current_pos = open_set[0];
			for (const auto cell_pos : open_set)
			{	// all open_set elements have f_score mapped
				if (f_score[cell_pos] < f_score[current_pos])
					current_pos = cell_pos;
			}

			if (current_pos == end)
				return;

			// A erosion
			// Erosion here is very large scale and interesting
			cave.get_cell(current_pos).reduce_density(data.erosion.erosion_a);

			open_set.erase(std::find(open_set.begin(), open_set.end(), current_pos)); // is found
			for (const auto neighbor_pos : cave.get_nearby_positions(current_pos, 1.5))
			{
				// Clamp density, otherwise goes crazy
				const double density = std::max(0.0, cave.get_cell(neighbor_pos).get_density());
				double tentative_g_score = g_score[current_pos] + density * density;

				// if neighbor doesnt have a g_score, init it to inf
				if (g_score.count(neighbor_pos) == 0)
					g_score[neighbor_pos] = std::numeric_limits<double>::infinity();

				// B erosion
				// erosion here will make smooth cave like C
				cave.get_cell(neighbor_pos).reduce_density(data.erosion.erosion_b);

				if (tentative_g_score < g_score[neighbor_pos])
				{
					// C erosion
					// same effect as B
					cave.get_cell(neighbor_pos).reduce_density(data.erosion.erosion_c);

					g_score[neighbor_pos] = tentative_g_score;
					f_score[neighbor_pos] = tentative_g_score +
						cave.distance(neighbor_pos, end) *
						Random::randreal(0.9,1.0);
					auto it = std::find(open_set.begin(), open_set.end(), neighbor_pos);
					if (it == open_set.end())
						open_set.push_back(neighbor_pos);
				}
			}
		}
	}
	void smooth_terrain(Data& data)
	{
		echo("Smoothing terrain...");
		// Set each density to the average within a radius
		const auto radius = 1;
		const auto intensity = data.smooth.intensity;
		const auto iterations = data.smooth.iterations;

		for (size_t i = 0; i < iterations; ++i)
		{
			std::vector<double> densities;
			densities.reserve(data.cave.get_area());

			for (const auto& pos : data.cave.get_positions())
			{	// calculate average of surrounding cells density
				double avg = data.cave.get_cell(pos).get_density();
				const auto nearby_positions = data.cave.get_nearby_positions(pos, radius, Cell::Type::Floor);
				for (const auto& position : nearby_positions)
					avg += data.cave.get_cell(position).get_density();
				avg /= 1 + nearby_positions.size();
				densities.push_back(avg);
			}

			for (size_t j = 0; j < data.cave.get_area(); ++j)
			{
				auto& cell = data.cave.get_cell({j, data.cave.get_idx()});
				if (data.smooth.rock == false && cell.get_type() == Cell::Type::Rock)
					continue;
				if (cell.get_type() == Cell::Type::Source || cell.get_type() == Cell::Type::Sink)
					continue;
				const double density = cell.get_density();
				const double diff = densities[j] - density;
				cell.set_density(density + diff * intensity);
			}
		}
	}



	void generate(Data& data)
	{
		set_rock_densities(data);
		set_water_features(data);
		form_tunnels(data);
		set_entities(data);
		render(data);
	}

	void generate_cave(Data& data)
	{
		generate(data);
		Dialog::get_selection("Cave ready", {"OK"});
	}
}
