#include "systems/environment/LiquidSystem.hpp"

#include <algorithm>
#include <vector>
#include "domain/Cave.hpp"
#include "domain/Cell.hpp"
#include "domain/LiquidMixture.hpp"
#include "domain/Liquid.hpp"
#include "utils/Random.hpp"

namespace LiquidSystem
{
	void simulate_liquid_flow(Cave& cave)
	{
		auto& cells = cave.get_cells();
		auto floor_cells = cave.get_cells_with_type(Cell::Type::Floor);
		std::shuffle(floor_cells.begin(), floor_cells.end(), Random::rng());
		for (const auto middle_idx : floor_cells)
		{
			auto& middle_cell = cells[middle_idx];
			auto& middle_mix = middle_cell.get_liquid_mixture();
			if (middle_mix.get_volume() < middle_mix.get_viscosity())
				continue;

			auto neighbors = cave.get_nearby_ids(middle_idx, 1.5, Cell::Type::Floor);
			std::sort(neighbors.begin(), neighbors.end(),
					[&](const auto a, const auto b)
					{
					return cells[a].get_liquid_level() < cells[b].get_liquid_level();
					});

			for (const auto neighbor_idx : neighbors)
			{
				auto& neighbor = cells[neighbor_idx];
				auto& neighbor_mix = neighbor.get_liquid_mixture();
				const double diff = middle_cell.get_liquid_level() - neighbor.get_liquid_level();
				if (diff <= 0 || middle_mix.get_volume() == 0)
					break;

				LiquidMixture flow = middle_mix.flow(diff / 2);
				neighbor_mix += flow;
			}
		}
		for (const auto idx : cave.get_sinks())
			cave.get_cell(idx).clear_liquids();
	}
	void simulate_liquid_diffusion(Cave& cave)
	{
		auto& cells = cave->get_cells();
		auto floor_cells = cave->get_cells_with_type(Cell::Type::Floor);
		std::shuffle(floor_cells.begin(), floor_cells.end(), Random::rng());
		for (const auto middle_idx : floor_cells)
		{
			auto& middle_cell = cells[middle_idx];
			auto& middle_mix = middle_cell.get_liquid_mixture();
			if (middle_mix.get_volume() <= middle_mix.get_viscosity())
				continue;

			const auto neighbors = cave->get_nearby_ids(middle_idx, 1.5, Cell::Type::Floor);
			auto& neighbor_mix = cells[neighbors[neighbors.size() - 1]].get_liquid_mixture();
			if (neighbor_mix.get_volume() <= neighbor_mix.get_viscosity())
				continue;

			const double volume = std::min(middle_mix.get_volume(), neighbor_mix.get_volume()) / 10;
			neighbor_mix += middle_mix.flow(volume);
			middle_mix += neighbor_mix.flow(volume);
		}
	}

	void simulate_liquid_sources(Cave& cave)
	{
		const auto sources = cave.get_cells_with_type(Cell::Type::Source);
		for (const auto source : sources)
		{
			auto& cell = cave.get_cell(source);
			auto& lm = cell.get_liquid_mixture();

			// liquid source is both the types and amount added each iteration
			// the values are set in CaveGenerator
			auto liquid_source = cell.get_liquid_source();
			const auto rate = liquid_source.get_volume();
			lm += liquid_source.flow(rate);
		}
	}

	void simulate_liquids(Cave& cave)
	{
		simulate_liquid_flow(cave);
		simulate_liquid_diffusion(cave);
		simulate_liquid_sources(cave);
	}
	double get_liquids_volume(const entt::registry& registry, const size_t cave_idx)
	{
		double volume = 0;
		const auto& cave = PositionSystem::get_cave(registry, cave_idx);
		const auto& cells = cave.get_cells();
		for (const auto& cell : cells)
			volume += cell.get_liquid_mixture().get_volume();
		return volume;
	}
};
