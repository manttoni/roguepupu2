#include "external/entt/entt.hpp"
#include "utils/ECS.hpp"
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
		if (ECS::get_cell(registry, position).get_type() == Cell::Type::Rock)
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
		const Cave& cave = ECS::get_cave(registry, a);
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
		const double distance = ECS::distance(registry, entity_pos, position);
		const double vision_range = StateSystem::get_vision_range(registry, entity);

		return los && distance <= vision_range;
	}

	std::vector<Position> get_visible_positions(const entt::registry& registry, const entt::entity entity)
	{
		if (!registry.all_of<Perception, Position>(entity))
			return {};

		const auto& pos = registry.get<Position>(entity);
		std::vector<Position> visible_positions = {pos};
		const auto& cave = ECS::get_cave(registry, pos);
		const auto vision_range = StateSystem::get_vision_range(registry, entity);
		for (const auto nearby_pos : cave.get_nearby_positions(pos, vision_range))
		{
			if (has_line_of_sight(registry, pos, nearby_pos))
				visible_positions.push_back(nearby_pos);
		}
		return visible_positions;
	}

	std::vector<entt::entity> get_visible_entities_in_position(const entt::registry& registry, const entt::entity seer, const Position& position)
	{
		std::vector<entt::entity> visible_entities;
		for (const auto entity : ECS::get_entities(registry, position))
		{
			// Skip adding if it's not visible, unless it's seer. (Always see yourself)
			if (registry.any_of<Hidden, Invisible>(entity) && entity != seer)
				continue;
			visible_entities.push_back(entity);
		}
		return visible_entities;
	}
	std::vector<entt::entity> get_visible_entities(const entt::registry& registry, const entt::entity seer)
	{
		std::vector<entt::entity> visible_entities;
		for (const auto position : get_visible_positions(registry, seer))
		{
			const auto& in_position = get_visible_entities_in_position(registry, seer, position);
			visible_entities.insert(visible_entities.end(), in_position.begin(), in_position.end());
		}
		return visible_entities;
	}
};
