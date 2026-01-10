#include "systems/EnvironmentSystem.hpp"

#include <algorithm>
#include <vector>
#include "Cave.hpp"
#include "Cell.hpp"
#include "World.hpp"
#include "Utils.hpp"
#include "entt.hpp"
#include "ECS.hpp"

namespace EnvironmentSystem
{
	void simulate_environment(Cave* cave)
	{
		simulate_liquids(cave);
	}
	void simulate_liquids(Cave* cave)
	{
		if (cave == nullptr) return;
		auto& cells = cave->get_cells();
		auto floor_cells = cave->get_cells_with_type(Cell::Type::Floor);
		std::sort(floor_cells.begin(), floor_cells.end(),
				[&](const auto a, const auto b)
				{
				return cells[a].get_liquid_level() > cells[b].get_liquid_level();
				});

		for (const auto middle_idx : floor_cells)
		{
			auto& middle_cell = cells[middle_idx];
			auto& middle_mix = middle_cell.get_liquid_mixture();
			if (middle_mix.get_volume() == 0)
				continue;

			auto neighbors = cave->get_nearby_ids(middle_idx, 1.5, Cell::Type::Floor);
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
		const auto& registry = cave->get_world()->get_registry();
		const auto sink = ECS::get_sink(registry, *cave);
		Cell* sink_cell = ECS::get_cell(registry, sink);
		sink_cell->clear_liquids();
	}

	double get_liquids_volume(Cave* cave)
	{
		double volume = 0;
		const auto& cells = cave->get_cells();
		for (const auto& cell : cells)
			volume += cell.get_liquid_mixture().get_volume();
		return volume;
	}
};
