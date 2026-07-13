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
	static constexpr double GRAVITY = 1.0;

	void simulate_liquid_flow(entt::registry& registry, const size_t cave_idx)
	{
		auto& cave = ECS::get_cave(registry, cave_idx);
		for (size_t i = 0; i < cave.get_area(); ++i)
		{
			auto& source_cell = cave.get_cell(i);
			auto& source_lm = source_cell.get_liquid_mixture();
			if (source_lm.empty() || source_cell.get_type() != Cell::Type::Floor)
				continue;
			Vec2<double> source = Vec2<double>::from_idx(i, cave.get_size());
			Vec2<double> momentum = source_lm.get_momentum();
			Vec2<double> flow_direction = cave.flow_direction(i);
			Vec2<double> direction = momentum + flow_direction * GRAVITY; // Might have to normalize?
			Vec2<double> destination = source + direction.normalize();
			destination = destination.round();

			if (destination.out_of_bounds(0, cave.get_size() - 1) || source == destination)
				continue;

			auto& destination_cell = cave.get_cell(destination.to_idx(cave.get_size()));
			if (destination_cell == source_cell)
				continue;
			const auto volume = direction.length();
			auto& destination_lm = destination_cell.get_liquid_mixture();
			destination_lm += source_lm.flow(volume);
		}
	}

	void simulate_liquid_sources(entt::registry& registry, const size_t cave_idx)
	{
		auto& cave = ECS::get_cave(registry, cave_idx);
		const auto liquid_sources = ECS::get_entities<LiquidSource>(registry, cave_idx);
		for (const auto source : liquid_sources)
		{
			auto& component = registry.get<LiquidSource>(source);
			auto& pos = registry.get<Position>(source);
			auto& cell = cave.get_cell(pos);
			auto& cell_lm = cell.get_liquid_mixture();
			auto& source_lm = component.liquid_mixture;
			cell_lm += source_lm.flow(component.flow_rate);
			if (component.liquid_mixture.empty())
				registry.remove<LiquidSource>(source);
		}
	}

	void simulate_liquids(entt::registry& registry, size_t cave_idx)
	{
		if (cave_idx == Position::invalid_idx)
			cave_idx = ECS::get_active_cave(registry).get_idx();
		simulate_liquid_flow(registry, cave_idx);
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
