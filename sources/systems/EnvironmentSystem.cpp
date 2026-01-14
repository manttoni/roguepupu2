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
	void simulate_liquid_flow(const entt::registry& registry, const size_t cave_idx)
	{
		auto& cave = PositionSystem::get_cave(registry, cave_idx);
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
			ECS::get_cell(registry, {idx, cave.get_idx()}).clear_liquids();
	}
	void simulate_liquid_diffusion(const entt::registry& registry, const size_t cave_idx)
	{
		auto& cave = PositionSystem::get_cave(registry, cave_idx);
		auto& cells = cave->get_cells();
		auto floor_cells = cave->get_cells_with_type(Cell::Type::Floor);
		std::shuffle(floor_cells.begin(), floor_cells.end(), Random::rng());
		for (const auto middle_idx : floor_cells)
		{
			auto& middle_cell = cells[middle_idx];
			auto& middle_mix = middle_cell.get_liquid_mixture();
			if (middle_mix.get_volume() <= 0.01)
				continue;

			const auto neighbors = cave->get_nearby_ids(middle_idx, 1.5, Cell::Type::Floor);
			auto& neighbor_mix = cells[neighbors[neighbors.size() - 1]].get_liquid_mixture();
			if (neighbor_mix.get_volume() <= 0.01)
				continue;

			const double volume = std::min(middle_mix.get_volume(), neighbor_mix.get_volume()) / 10;
			neighbor_mix += middle_mix.flow(volume);
			middle_mix += neighbor_mix.flow(volume);

		}
	}
	void simulate_liquid_sources(const entt::registry& registry, const size_t cave_idx)
	{
		auto liquid_sources = registry.view<LiquidSource, Position>();
		for (const auto liquid_source : liquid_sources)
		{
			if (ECS::get_cave(registry, liquid_source) != cave)
				continue;
			auto& ls_component = registry.get<LiquidSource>(liquid_source);
			if (ls_component.volume_left == 0 || ls_component.type == Liquid::Type::None)
				continue;

			Cell& source_cell = PositionSystem::get_cell(registry, liquid_source);
			auto& lm = source_cell.get_liquid_mixture();
			const double volume_created = std::min(ls_component.rate, ls_component.volume_left);
			lm.add_liquid(ls_component.type, std::min(ls_component.rate, volume_created));
			ls_component.volume_left -= volume_created;
		}
	}

	void simulate_condensation(const entt::registry& registry, const size_t cave_idx)
	{
		auto& cave = PositionSystem::get_cave(registry, cave_idx);
		const auto floor_cells = cave.get_cells_with_type(Cell::Type::Floor);
		auto& cells = cave.get_cells();
		for (const auto idx : floor_cells)
		{
			Cell& cell = cells[idx];
			auto& lm = cell.get_liquid_mixture();
			lm.add_liquid(Liquid::Type::Water, pow(cell.get_humidity(), 64));
		}
	}
	void simulate_liquids(const entt::registry& registry, const size_t cave_idx)
	{
		simulate_liquid_flow(registry, cave_idx);
		simulate_liquid_diffusion(registry, cave_idx);
		simulate_liquid_sources(registry, cave_idx);
		simulate_condensation(registry, cave_idx);
	}
	void simulate_environment(const entt::registry& registry, const size_t cave_idx)
	{
		simulate_liquids(registry, cave_idx);
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
