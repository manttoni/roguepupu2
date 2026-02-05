#include "systems/environment/LiquidSystem.hpp"

#include <algorithm>
#include <vector>

#include "domain/Cave.hpp"
#include "domain/Cell.hpp"
#include "domain/LiquidMixture.hpp"
#include "utils/Random.hpp"
#include "utils/ECS.hpp"

namespace LiquidSystem
{
	void simulate_liquid_flow(entt::registry& registry, const size_t cave_idx)
	{
		auto& cave = ECS::get_cave(registry, cave_idx);
		auto positions = cave.get_positions();
		std::shuffle(positions.begin(), positions.end(), Random::rng());
		std::vector<bool> flowed(cave.get_area(), false);
		for (const auto middle_pos : positions)
		{
			auto& middle_cell = cave.get_cell(middle_pos);
			auto& middle_mix = middle_cell.get_liquid_mixture();
			if (middle_mix.get_volume() == 0)
				continue;

			auto neighbors = cave.get_nearby_positions(middle_pos, 1.5);
			std::sort(neighbors.begin(), neighbors.end(),
					[&](const auto a, const auto b)
					{
					return ECS::get_liquid_level(registry, a) < ECS::get_liquid_level(registry, b);
					});

			for (const auto neighbor_pos : neighbors)
			{
				auto& neighbor = cave.get_cell(neighbor_pos);
				if (neighbor.get_type() == Cell::Type::Rock) continue; // dont simulate liquids for rock cells
				auto& neighbor_mix = neighbor.get_liquid_mixture();
				const double diff = ECS::get_liquid_level(registry, middle_pos) - ECS::get_liquid_level(registry, neighbor_pos);
				if (diff <= 0 || middle_mix.get_volume() == 0)
					break;

				if (flowed[neighbor_pos.cell_idx])
					continue;

				LiquidMixture flow = middle_mix.flow(diff / 2);
				neighbor_mix += flow;

				//flowed[neighbor_pos.cell_idx] = true; // will this make it look more "flowy"?
			}
		}
		for (const auto sink_pos : cave.get_positions_with_type(Cell::Type::Sink))
			cave.get_cell(sink_pos).clear_liquids();
	}
	void simulate_liquid_diffusion(entt::registry& registry, const size_t cave_idx)
	{
		auto& cave = ECS::get_cave(registry, cave_idx);
		auto floor_cells = cave.get_positions_with_type(Cell::Type::Floor);
		std::shuffle(floor_cells.begin(), floor_cells.end(), Random::rng());
		for (const auto middle_pos : floor_cells)
		{
			auto& middle_cell = cave.get_cell(middle_pos);
			auto& middle_mix = middle_cell.get_liquid_mixture();
			if (middle_mix.get_volume() == 0)
				continue;

			const auto neighbors = cave.get_nearby_positions(middle_pos, 1.5, Cell::Type::Floor);
			if (neighbors.empty()) continue;
			auto& neighbor_mix = cave.get_cell(neighbors.front()).get_liquid_mixture();
			if (neighbor_mix.get_volume() == 0)
				continue;

			const double volume = std::min(middle_mix.get_volume(), neighbor_mix.get_volume()) / 10;
			neighbor_mix += middle_mix.flow(volume);
			middle_mix += neighbor_mix.flow(volume);
		}
	}

	void simulate_liquid_sources(entt::registry& registry, const size_t cave_idx)
	{
		auto& cave = ECS::get_cave(registry, cave_idx);
		const auto sources = cave.get_positions_with_type(Cell::Type::Source);
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

	void simulate_liquids(entt::registry& registry, size_t cave_idx)
	{
		if (cave_idx == Position::invalid_idx)
			cave_idx = ECS::get_active_cave(registry).get_idx();
		simulate_liquid_flow(registry, cave_idx);
		simulate_liquid_diffusion(registry, cave_idx);
		simulate_liquid_sources(registry, cave_idx);
	}
	double get_liquids_volume(const entt::registry& registry, const size_t cave_idx)
	{
		double volume = 0;
		const auto& cave = ECS::get_cave(registry, cave_idx);
		const auto& cells = cave.get_cells();
		for (const auto& cell : cells)
			volume += cell.get_liquid_mixture().get_volume();
		return volume;
	}
};
