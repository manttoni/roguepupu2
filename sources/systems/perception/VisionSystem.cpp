#include "external/entt/entt.hpp"
#include "utils/ECS.hpp"
#include "systems/state/StateSystem.hpp"
#include "components/Components.hpp"
#include "domain/Cell.hpp"

namespace VisionSystem
{
	/* Return amount of blocked vision
	 * Opaqueness is opposite of transparency
	 * */
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

	/* Return true if opaqueness reaches 1.0
	 * Some obstacles can be transparent
	 * This is used also for light rays
	 * Digital Differential Analyzer
	 * */
	bool has_line_of_sight(const entt::registry& registry, const Position& a, const Position& b)
	{
		if (a.cave_idx != b.cave_idx)
			return false;

		const Cave& cave = ECS::get_cave(registry, a.cave_idx);

		size_t size = cave.get_size();
		double x0 = static_cast<double>(a.cell_idx % size) + 0.5;
		double y0 = static_cast<double>(a.cell_idx / size) + 0.5;
		double x1 = static_cast<double>(b.cell_idx % size) + 0.5;
		double y1 = static_cast<double>(b.cell_idx / size) + 0.5;

		double dx = x1 - x0;
		double dy = y1 - y0;

		int steps = static_cast<int>(std::max(std::abs(dx), std::abs(dy)));

		if (steps == 0)
			return true;

		double x_inc = dx / steps;
		double y_inc = dy / steps;

		double x = x0;
		double y = y0;

		double opaqueness = 0.0;

		for (int i = 0; i <= steps; ++i)
		{
			int ix = static_cast<int>(x);
			int iy = static_cast<int>(y);
			size_t idx = iy * size + ix;

			// If endpoint is rock, it should be visible. Same with start, to be symmetrical
			if (idx != a.cell_idx && idx != b.cell_idx)
				opaqueness += VisionSystem::get_opaqueness(registry, Position(idx, a.cave_idx));

			x += x_inc;
			y += y_inc;
		}

		return opaqueness < 1.0;
	}

	/* Return true if entity can see position
	 * */
	bool has_vision(const entt::registry& registry, const entt::entity entity, const Position& target_pos)
	{
		if (!registry.all_of<Position, Perception>(entity))
			return false;
		const Position& entity_pos = registry.get<Position>(entity);
		if (entity_pos.cave_idx != target_pos.cave_idx)
			return false;

		const auto vision_range = StateSystem::get_vision_range(registry, entity);
		if (vision_range < ECS::distance(registry, entity_pos, target_pos))
			return false;
		if (!has_line_of_sight(registry, entity_pos, target_pos))
			return false;
		return true;
	}

	/* Get all positions entity can see
	 * */
	std::vector<Position> get_visible_positions(const entt::registry& registry, const entt::entity entity)
	{
		if (!registry.all_of<Perception, Position>(entity))
			return {};

		const auto& entity_pos = registry.get<Position>(entity);
		assert(entity_pos.is_valid());
		std::vector<Position> visible_positions;
		const auto& cave = ECS::get_cave(registry, entity_pos);
		for (const auto pos : cave.get_positions())
		{
			if (has_vision(registry, entity, pos))
				visible_positions.push_back(pos);
		}
		return visible_positions;
	}

	/* Get list of entities "seer" can see in a position
	 * */
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

	/* Get list of entities "seer" can see
	 * */
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
