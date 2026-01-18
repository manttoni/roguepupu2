#include <algorithm>
#include <cassert>
#include <vector>
#include <limits>
#include "utils/Math.hpp"
#include "domain/LiquidMixture.hpp"
#include "generation/CaveGenerator.hpp"
#include "domain/Cave.hpp"
#include "domain/Cell.hpp"
#include "systems/environment/LiquidSystem.hpp"
#include "systems/position/MovementSystem.hpp"
#include "utils/Random.hpp"

namespace CaveGenerator
{

	bool is_on_edge(const Data& data, const Vec2 coords)
	{
		const auto margin_size = static_cast<int>(data.margin.size);
		const auto cave_size = static_cast<int>(data.cave.get_size());
		return
			coords.y < margin_size ||
			coords.x < margin_size ||
			coords.y > cave_size - margin_size ||
			coords.x > cave_size - margin_size;
	}

	/* Give each cell a density [1,9] from Perlin Noise
	 * Make edges denser to keep erosion_simulation away from them
	 * */
	void set_rock_densities(Data& data)
	{
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
	 * Set +inf for sources, which is probably a bad idea
	 * */
	void set_water_features(Data& data)
	{
		const size_t cave_size = data.cave.get_size();
		std::vector<size_t> indices(cave_size * cave_size);
		std::iota(indices.begin(), indices.end(), 0);
		std::shuffle(indices.begin(), indices.end(), Random::rng());

		size_t spawned_sources = 0, spawned_sinks = 0;
		for (const auto idx : indices)
		{
			const Vec2 coords(idx, cave_size);
			if (is_on_edge(data, coords))
				continue;

			if (spawned_sources < data.features.sources)
			{
				data.cave.get_cell(idx).set_density(std::numeric_limits<double>::infinity());
				spawned_sources++;
				continue;
			}
			else if (spawned_sinks < data.features.sinks)
			{
				data.cave.get_cell(idx).set_density(-std::numeric_limits<double>::infinity());
				spawned_sinks++;
				continue;
			}
			else
				break;
		}
		assert(spawned_sources == data.features.sources && spawned_sinks == data.features.sinks);
	}

	/* Simulate erosion until all sources and sinks are connected with a walkable path
	 * */
	void form_tunnels(Data& data)
	{
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
					erosion_simulation(data, source.cell_idx, sink.cell_idx);
					if (flag == true &&
						MovementSystem::find_path(data.registry, source, sink, false).empty())
						flag = false;
				}
			}
		}
	}

	// A* to find path of least resistance through solid rock
	void erosion_simulation(Data& data, const size_t start, const size_t end)
	{
		auto& cave = data.cave;
		auto& cells = cave.get_cells();

		std::vector<size_t> open_set = { start };
		std::map<size_t, size_t> came_from;

		std::map<size_t, double> g_score;
		std::map<size_t, double> f_score;
		g_score[start] = 0;
		f_score[start] = data.cave.distance(start, end);
		while (!open_set.empty())
		{
			size_t current_idx = open_set[0];
			for (const size_t cell_idx : open_set)
			{	// all open_set elements have f_score mapped
				if (f_score[cell_idx] < f_score[current_idx])
					current_idx = cell_idx;
			}

			if (current_idx == end)
				return;

			// A erosion
			// Erosion here is very large scale and interesting
			cells[current_idx].reduce_density(data.erosion.erosion_a);

			open_set.erase(std::find(open_set.begin(), open_set.end(), current_idx)); // is found
			for (const auto neighbor_pos : cave.get_nearby_positions(current_idx))
			{
				const size_t neighbor_idx = neighbor_pos.cell_idx;
				// Clamp density, otherwise goes crazy
				const double density = std::max(0.0, cells[neighbor_idx].get_density());
				double tentative_g_score = g_score[current_idx] + density * density;

				// if neighbor doesnt have a g_score, init it to inf
				if (g_score.count(neighbor_idx) == 0)
					g_score[neighbor_idx] = std::numeric_limits<double>::infinity();

				// B erosion
				// erosion here will make smooth cave like C
				cells[neighbor_idx].reduce_density(data.erosion.erosion_b);

				if (tentative_g_score < g_score[neighbor_idx])
				{
					// C erosion
					// same effect as B
					cells[neighbor_idx].reduce_density(data.erosion.erosion_c);

					came_from[neighbor_idx] = current_idx;
					g_score[neighbor_idx] = tentative_g_score;
					f_score[neighbor_idx] = tentative_g_score + cave.distance(neighbor_idx, end);
					auto it = std::find(open_set.begin(), open_set.end(), neighbor_idx);
					if (it == open_set.end())
						open_set.push_back(neighbor_idx);
				}
			}
		}
	}
/*
	void CaveGenerator::normalize_negative_density(Cave& cave) const
	{
		auto& cells = cave.get_cells();
		double deepest = std::numeric_limits<double>::infinity();
		for (const auto cell_idx : cave.get_cells_with_type(Cell::Type::Floor))
		{
			const double density = cells[cell_idx].get_density();
			deepest = std::min(deepest, density);
		}
		for (const auto cell_idx : cave.get_cells_with_type(Cell::Type::Floor))
		{
			const double density = cells[cell_idx].get_density();
			cells[cell_idx].set_density(Math::map(density, deepest, 0.0, -0.9, 0.0));
		}
	}
*/
	void smooth_terrain(Data& data)
	{
		// Set each density to the average within a radius
		const auto radius = data.smooth.intensity;
		const auto iterations = data.smooth.iterations;
		auto& cells = data.cave.get_cells();

		for (size_t i = 0; i < iterations; ++i)
		{
			std::vector<double> densities;
			densities.reserve(data.cave.get_area());

			for (size_t j = 0; j < data.cave.get_area(); ++j)
			{	// calculate average of surrounding cells density
				double avg = cells[j].get_density();
				const auto nearby_positions = data.cave.get_nearby_positions(j, radius);
				for (const auto& position : nearby_positions)
					avg += cells[position.cell_idx].get_density();
				avg /= 1 + nearby_positions.size();
				densities.push_back(avg);
			}

			for (size_t j = 0; j < data.cave.get_area(); ++j)
				cells[j].set_density(densities[j]);
		}
	}
/*
	void CaveGenerator::add_water(Cave& cave) const
	{
		auto& cells = cave.get_cells();
		for (const auto cell_idx : cave.get_cells_with_type(Cell::Type::Floor))
		{
			auto& cell = cells[cell_idx];
			const double amount = Random::randreal(0, 2 * conf.average_water);
			auto& mix = cell.get_liquid_mixture();
			mix.add_liquid(Liquid::Type::Water, amount);
		}
	}
*/

	/* Simulate liquids until the volume doesn't change significantly
	 * It might be very slow to fill the cave in some cases. Use add_water if needed
	 * */
	void stabilize_liquids(Data& data)
	{
		const size_t max_rounds = 10000;
		const double significant_diff = 0.1;
		double liquids_volume = LiquidSystem::get_liquids_volume(data.registry, data.cave.get_idx());
		for (size_t i = 0; i < max_rounds; ++i)
		{
			LiquidSystem::simulate_liquids(data.registry, data.cave.get_idx());
			const double next_liquids_volume = LiquidSystem::get_liquids_volume(data.registry, data.cave.get_idx());
			if (abs(liquids_volume - next_liquids_volume) < significant_diff)
				break;
		}
	}


	void generate_cave(Data& data)
	{
		set_rock_densities(data);
		set_water_features(data);
		form_tunnels(data);
		smooth_terrain(data);
		//add_water(data);
		stabilize_liquids(data);
	}
}
