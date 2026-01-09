#include "systems/EnvironmentSystem.hpp"

#include <algorithm>
#include "Cave.hpp"
#include "Utils.hpp"
#include "entt.hpp"
#include "ECS.hpp"

namespace EnvironmentSystem
{
	void simulate_environment(entt::registry& registry)
	{
		simulate_liquids(registry);
	}
	void simulate_liquids(entt::registry& registry)
	{
		Cave* cave = ECS::get_active_cave(registry);
		auto& cells = cave->get_cells();
		auto floor_cells = cave->get_cells_with_type(Cell::Type::Floor);
		std::shuffle(floor_cells.begin(), floor_cells.end(), Random::rng());
		for (const auto cell_idx : floor_cells)
		{
			auto nearby_floor = cave->get_nearby_ids(cell_idx, 1.5, Cell::Type::Floor);
			if (nearby_floor.empty())
				continue;
			std::sort(nearby_floor.begin(), nearby_floor.end(),
					[&](const auto& a, const auto& b)
					{
						return cells[a].get_liquid_level() < cells[b].get_liquid_level();
					});
			double total = cells[cell_idx].get_liquid_level();
			for (const auto neighbor_idx : nearby_floor)
				total += cells[neighbor_idx].get_liquid_level();
			const double average = total / (1 + nearby_floor.size());

			// Collect excess from middle cell
			std::map<Liquid::Type, double> excess;
			Cell& cell = cells[cell_idx];
			while (cell.get_liquid_level() > average && !cell.get_liquids().empty())
			{
				const auto cell_liquids = cell.get_liquids();
				Liquid::Type type = cell_liquids.begin()->first;
				excess[type] += cell.remove_liquid(type, cell.get_liquid_level() - average);
			}

			// Distribute excess to neighbors
			for (const auto neighbor_idx : nearby_floor)
			{
				Cell& cell = cells[neighbor_idx];
				if (cell.get_liquid_level() < average && !excess.empty())
				{
					for (auto& [excess_type, excess_amount] : excess)
					{
						if (excess_amount <= 0.0)
						{
							excess.erase(excess_type);
							break;
						}
						const double amount = std::min(excess_amount, average - cell.get_liquid_level());
						cell.add_liquid(excess_type, amount);
						excess[excess_type] -= amount;
					}
				}
			}

			// Add rest back to middle in case some are left
			for (const auto& [type, amount] : excess)
				cells[cell_idx].add_liquid(type, amount);

		}

		// Remove all liquids on sink
		const auto sink = ECS::get_sink(registry, *cave);
		Cell* sink_cell = ECS::get_cell(registry, sink);
		sink_cell->clear_liquids();
	}
};
