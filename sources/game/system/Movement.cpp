#include "game/system/Movement.hpp"
#include "game/Query.hpp"
#include "game/entity/Entity.hpp"
#include "utils/Vec2.hpp"
#include "game/world/Position.hpp"

namespace
{
	// TODO: Extract shared spatial queries when needed.
	bool blocks_movement(
		const entt::registry& registry,
		const Game::World::Grid& grid,
		const Game::World::GlobalPosition& position)
	{
		const auto* cell = grid.find_cell(position);

		if (!cell || cell->form != Game::Enum::Form::Floor)
			return true;

		const auto entities = Game::Query::find_all(
			registry,
			Game::Component::Value::Position{position},
			Game::Component::Value::CollisionMovement{true}
			);

		return !entities.empty();
	}

	double distance(
		const Game::World::GlobalPosition& from,
		const Game::World::GlobalPosition& to)
	{
		const auto& diff = from.vec2() - to.vec2();
		return diff.length();
	}
}

namespace Game::System::Movement
{
	double cost(
		const Game::Simulation& simulation,
		const World::GlobalPosition& from,
		const World::GlobalPosition& to)
	{
		(void) simulation;
		return distance(from, to);
	}

	bool has_path(
		const entt::registry& registry,
		const World::Grid& grid,
		const World::GlobalPosition& from,
		const World::GlobalPosition& to)
	{
		const auto dx = std::abs(static_cast<double>(to.x) - from.x);
		const auto dy = std::abs(static_cast<double>(to.y) - from.y);

		if (dx > 1.0 || dy > 1.0)
			return false;

		if (blocks_movement(registry, grid, to))
			return false;

		if (dx == 0.0 || dy == 0.0)
			return true;

		const auto corner1 = World::GlobalPosition{from.y, to.x};
		const auto corner2 = World::GlobalPosition{to.y, from.x};
		return !blocks_movement(registry, grid, corner1)
			&& !blocks_movement(registry, grid, corner2);
	}
}

