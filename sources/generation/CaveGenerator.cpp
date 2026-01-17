#include <algorithm>
#include <cassert>
#include <vector>
#include <limits>
#include "utils/Math.hpp"
#include "systems/position/PositionSystem.hpp"
#include "domain/LiquidMixture.hpp"
#include "generation/CaveGenerator.hpp"
#include "domain/Cave.hpp"
#include "domain/Cell.hpp"
#include "systems/environment/LiquidSystem.hpp"
#include "systems/position/MovementSystem.hpp"
#include "utils/Random.hpp"

CaveGenerator::CaveGenerator(const GenerationConf& conf) : conf(conf) {}

void CaveGenerator::set_rock_densities(Cave& cave) const
{
	auto& cells = cave.get_cells();
	const size_t seed = Random::randsize_t(0, 99999);
	for (auto& cell : cells)
	{
		const size_t idx = cell.get_idx();
		const size_t y = idx / conf.size;
		const size_t x = idx % conf.size;
		const double perlin = Random::noise2D(y, x, conf.frequency, conf.octaves, seed);
		const double density = Math::map(perlin, 0, 1, 1, CELL_DENSITY_MAX);
		cell.set_density(density);
	}
}

void CaveGenerator::set_water_features(Cave& cave) const
{
	size_t sources = Random::randsize_t(1, conf.max_sources);
	size_t sinks = Random::randsize_t(1, conf.max_sinks);
	std::vector<size_t> indices(conf.size * conf.size);
	std::iota(indices.begin(), indices.end(), 0);
	std::shuffle(indices.begin(), indices.end(), Random::rng());

	for (const auto idx : indices)
	{
		const size_t y = idx / conf.size;
		const size_t x = idx % conf.size;
		if (y < conf.margin || y > conf.size - conf.margin ||
			x < conf.margin || x > conf.size - conf.margin)
			continue;
		else if (sources > 0)
		{
			cave.get_cell(idx).set_density(std::numeric_limits<double>::infinity());
			sources--;
			continue;
		}
		else if (sinks > 0)
		{
			cave.get_cell(idx).set_density(-std::numeric_limits<double>::infinity());
			sinks--;
			continue;
		}
		else
			break;
	}
}

void CaveGenerator::form_tunnels(Cave& cave) const
{
	const auto& sources = cave.get_cells_with_type(Cell::Type::Source);
	const auto& sinks = cave.get_cells_with_type(Cell::Type::Sink);

	bool flag = false;
	while (flag == false)
	{
		flag = true;
		for (const auto source : sources)
		{
			for (const auto sink : sinks)
			{
				erosion_simulation(cave, source, sink);
				if (flag == true && MovementSystem::find_path(cave, source, sink, false).empty())
					flag = false;
			}
		}
	}
}

// A* to find path of least resistance through solid rock
void CaveGenerator::erosion_simulation(Cave& cave, const size_t start, const size_t end) const
{
	auto& cells = cave.get_cells();

	std::vector<size_t> open_set = { start };
	std::map<size_t, size_t> came_from;

	std::map<size_t, double> g_score;
	std::map<size_t, double> f_score;
	g_score[start] = 0;
	f_score[start] = PositionSystem::distance(cave, start, end);
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
		cells[current_idx].reduce_density(conf.erosion_a);

		open_set.erase(std::find(open_set.begin(), open_set.end(), current_idx)); // is found
		for (const size_t neighbor_idx : cave.get_nearby_ids(current_idx))
		{
			// Clamp density, otherwise goes crazy
			const double density = std::max(0.0, cells[neighbor_idx].get_density());
			double tentative_g_score = g_score[current_idx] + density * density;

			// if neighbor doesnt have a g_score, init it to inf
			if (g_score.count(neighbor_idx) == 0)
				g_score[neighbor_idx] = std::numeric_limits<double>::infinity();

			// B erosion
			// erosion here will make smooth cave like C
			cells[neighbor_idx].reduce_density(conf.erosion_b);

			if (tentative_g_score < g_score[neighbor_idx])
			{
				// C erosion
				// same effect as B
				cells[neighbor_idx].reduce_density(conf.erosion_c);

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

void CaveGenerator::smooth_terrain(Cave& cave) const
{
	(void) cave;
	// Old version was bad, invent something good here
}

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

void CaveGenerator::simulate_environment(Cave& cave, const size_t rounds) const
{
	for (size_t i = 0; i < rounds; ++i)
		LiquidSystem::simulate_liquids(cave);
}

/*Not important right now
 * void CaveGenerator::simulate_creatures(const Cave& cave, const size_t rounds) const
{
	std::vector<entt::entity> actors;
	for (const auto entity : registry.view<Position>())
	{
		if (registry.get<Position>(entity).cave == cave &&
			registry.get<Category>(entity).category == "creatures")
			actors.push_back(entity);
	}
	for (size_t i = 0; i < rounds; ++i)
	{
		for (const auto actor : actors)
		{

		}
	}
}*/

Cave CaveGenerator::generate() const
{
	Cave cave(conf.size);
	set_rock_densities(cave);
	set_water_features(cave);
	form_tunnels(cave);
//	normalize_negative_density(cave);
	add_water(cave);
	simulate_environment(cave);
	return cave;
}

