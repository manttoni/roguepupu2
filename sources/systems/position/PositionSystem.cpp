#include <cmath>
#include "utils/Vec2.hpp"
#include "systems/position/PositionSystem.hpp"
#include "domain/Cave.hpp"
#include "domain/Cell.hpp"
#include "utils/Log.hpp"
#include "domain/Position.hpp"
#include "external/entt/entt.hpp"
#include "domain/World.hpp"

namespace PositionSystem
{
	double distance(const Cave& cave, const Cell &a, const Cell &b)
	{
		return distance(cave, a.get_idx(), b.get_idx());
	}

	double distance(const Cave& cave, const size_t a, const size_t b)
	{
		const size_t size = cave.get_size();
		assert(a < size && b < size);
		const Vec2 start(a, size);
		const Vec2 end(b, size);
		return std::hypot(start.y - end.y, start.x - end.x);
	}

	double distance(const entt::registry& registry, const Position& a, const Position& b)
	{
		if (a.cave_idx != b.cave_idx)
			Log::error("Measuring distance between caves");
		const Cave& cave = get_cave(registry, a);
		return distance(cave, a.cell_idx, b.cell_idx);
	}
	double distance (const entt::registry& registry, const entt::entity a, const entt::entity b)
	{
		return distance(registry, registry.get<Position>(a), registry.get<Position>(b));
	}

	Cell& get_cell(entt::registry& registry, const Position& position)
	{
		return get_cave(registry, position).get_cell(position.cell_idx);
	}
	Cave& get_cave(entt::registry& registry, const Position& position)
	{
		return registry.ctx().get<World>().get_cave(position.cave_idx);
	}
	Cave& get_cave(entt::registry& registry, const size_t cave_idx)
	{
		return registry.ctx().get<World>().get_cave(cave_idx);
	}

	const Cell& get_cell(const entt::registry& registry, const Position& position)
	{
		return get_cave(registry, position).get_cell(position.cell_idx);
	}
	const Cave& get_cave(const entt::registry& registry, const Position& position)
	{
		return registry.ctx().get<World>().get_cave(position.cave_idx);
	}
	const Cave& get_cave(const entt::registry& registry, const size_t cave_idx)
	{
		return registry.ctx().get<World>().get_cave(cave_idx);
	}
};
