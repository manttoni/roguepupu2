#include "systems/PositionSystem.hpp"
#include "Cave.hpp"
#include "Cell.hpp"
#include "Utils.hpp"

namespace PositionSystem
{
	double distance(const Cave& cave, const Cell &a, const Cell &b)
	{
		return distance(cave, a.get_idx(), b.get_idx());
	}

	double distance(const Cave& cave, const size_t a, const size_t b)
	{
		int start_y = start_id / width;
		int start_x = start_id % width;
		int end_y = end_id / width;
		int end_x = end_id % width;

		return std::hypot(start_y - end_y, start_x - end_x);
	}

	double distance(const entt::registry& registry, const Position& a, const Position& b)
	{
		if (a.cave_idx() != b.cave_idx())
			Log::error("Measuring distance between caves");
		const Cave& cave = ECS::get_cave(registry, a);
		return distance(cave, a.cell_idx, b.cell_idx);
	}
	double distance (const entt::registry& registry, const entt::entity a, const entt::entity b)
	{
		return distance(registry, registry.get<Position>(a), registry.get<Position>(b));
	}

	Cell& get_cell(const entt::registry& registry, const Position& position)
	{
		return get_cave(registry, position).get_cell(position.cell_idx);
	}
	Cell& get_cell(const entt::registry& registry, const entt::entity entity)
	{
		return get_cell(registry, registry.get<Position>(entity));
	}
	Cave& get_cave(const entt::registry& registry, const Position& position)
	{
		return registry.ctx().get<World>().get_cave(position.cave_idx);
	}
	Cave& get_cave(const entt::registry& registry, const size_t cave_idx)
	{
		return registry.ctx().get<World>().get_cave(cave_idx);
	}
};
