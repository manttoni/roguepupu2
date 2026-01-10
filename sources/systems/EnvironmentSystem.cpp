#include "systems/EnvironmentSystem.hpp"

#include <algorithm>
#include <vector>
#include "Cave.hpp"
#include "Cell.hpp"
#include "World.hpp"
#include "Utils.hpp"
#include "entt.hpp"
#include "ECS.hpp"
#include "Components.hpp"

namespace EnvironmentSystem
{

	void simulate_liquids_flow(Cave* cave)
	{
		if (cave == nullptr) return;
		auto& cells = cave->get_cells();
		auto floor_cells = cave->get_cells_with_type(Cell::Type::Floor);
		/*std::sort(floor_cells.begin(), floor_cells.end(),
				[&](const auto a, const auto b)
				{
				return cells[a].get_liquid_level() > cells[b].get_liquid_level();
				});
		*/
		std::shuffle(floor_cells.begin(), floor_cells.end(), Random::rng());
		for (const auto middle_idx : floor_cells)
		{
			auto& middle_cell = cells[middle_idx];
			auto& middle_mix = middle_cell.get_liquid_mixture();
			if (middle_mix.get_volume() <= 0.01)
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
	void simulate_liquid_sources(Cave* cave)
	{
		auto& registry = cave->get_world()->get_registry();
		auto liquid_sources = registry.view<LiquidSource, Position>();
		for (const auto liquid_source : liquid_sources)
		{
			if (ECS::get_cave(registry, liquid_source) != cave)
				continue;
			auto& ls_component = registry.get<LiquidSource>(liquid_source);
			if (ls_component.volume_left == 0 || ls_component.type == Liquid::Type::None)
				continue;

			Cell* source_cell = ECS::get_cell(registry, liquid_source);
			auto& lm = source_cell->get_liquid_mixture();
			const double volume_created = std::min(ls_component.rate, ls_component.volume_left);
			lm.add_liquid(ls_component.type, std::min(ls_component.rate, volume_created));
			ls_component.volume_left -= volume_created;
		}
	}
	void simulate_condensation(Cave* cave)
	{
		const auto floor_cells = cave->get_cells_with_type(Cell::Type::Floor);
		auto& cells = cave->get_cells();
		for (const auto idx : floor_cells)
		{
			Cell& cell = cells[idx];
			auto& lm = cell.get_liquid_mixture();
			lm.add_liquid(Liquid::Type::Water, pow(cell.get_humidity(), 100));
		}
	}
	void simulate_liquids(Cave* cave)
	{
		simulate_liquids_flow(cave);
		simulate_liquid_sources(cave);
		simulate_condensation(cave);
	}
	void simulate_environment(Cave* cave)
	{
		simulate_liquids(cave);
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
