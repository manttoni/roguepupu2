#include "external/entt/entt.hpp"
#include "utils/ECS.hpp"
#include "systems/position/PositionSystem.hpp"
#include "systems/state/StateSystem.hpp"
#include "components/Components.hpp"
#include "domain/Cell.hpp"

namespace VisionSystem
{
	double get_vision_range(const entt::registry& registry, const entt::entity entity)
	{
		if (!registry.all_of<Vision>(entity))
			return 0;
		return registry.get<Vision>(entity).range;
	}

	double get_opaqueness(const entt::registry& registry, const Position& position)
	{
		if (PositionSystem::get_cell(registry, position).get_type() == Cell::Type::Rock)
			return 1.0;
		double opaqueness = 0.0;
		for (const auto entity : ECS::get_entities(registry, position))
		{
			if (registry.all_of<Opaque>(entity))
				opaqueness += registry.get<Opaque>(entity).value;
		}
		return opaqueness;
	}

	/* Return true if nothing blocks vision between */
	bool has_line_of_sight(const entt::registry& registry, const Position& a, const Position& b)
	{
		if (a.cave_idx != b.cave_idx)
			return false;
		const Cave& cave = PositionSystem::get_cave(registry, a);
		const auto start = a.cell_idx;
		const auto end = b.cell_idx;
		const size_t size = cave.get_size();
		int x0 = static_cast<int>(start % size);
		int y0 = static_cast<int>(start / size);
		int x1 = static_cast<int>(end % size);
		int y1 = static_cast<int>(end / size);

		int dx = abs(x1 - x0);
		int dy = abs(y1 - y0);

		int sx = x0 < x1 ? 1 : -1;
		int sy = y0 < y1 ? 1 : -1;

		int err = dx - dy;

		double opaqueness = 0.0;
		while (true)
		{
			size_t idx = y0 * size + x0;

			if (x0 == x1 && y0 == y1)
				break;

			opaqueness += VisionSystem::get_opaqueness(registry, Position(idx, a.cave_idx));
			if (opaqueness >= 1.0)
				return false;
			int e2 = 2 * err;
			if (e2 > -dy)
			{
				err -= dy;
				x0 += sx;
			}
			if (e2 < dx)
			{
				err += dx;
				y0 += sy;
			}
		}

		return true;
	}

	/* Retirn true if has line of sight and is within vision range */
	bool has_vision(const entt::registry& registry, const entt::entity entity, const Position& position)
	{
		if (!registry.all_of<Position, Perception>(entity))
			return false;
		const Position& entity_pos = registry.get<Position>(entity);
		const bool los = has_line_of_sight(registry, entity_pos, position);
		const double distance = PositionSystem::distance(registry, entity_pos, position);
		const double vision_range = StateSystem::get_vision_range(registry, entity);

		return los && distance <= vision_range;
	}

	std::vector<size_t> get_visible_cells(const entt::registry& registry, const entt::entity entity)
	{
		if (!registry.all_of<Perception>(entity))
			return {};
		std::vector<size_t> visible_cells;
		const auto& pos = registry.get<Position>(entity);
		const auto& cave = PositionSystem::get_cave(registry, pos);
		const auto vision_range = StateSystem::get_vision_range(registry, entity);
		for (const auto nearby_idx : cave.get_nearby_ids(pos.cell_idx, vision_range))
		{
			if (has_line_of_sight(registry, pos, Position(nearby_idx, cave.get_idx())))
				visible_cells.push_back(nearby_idx);
		}
		return visible_cells;
	}
};
