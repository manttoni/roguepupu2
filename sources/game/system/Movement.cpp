#include "game/system/Movement.hpp"
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

		const auto entities = Game::Entity::find_all(
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
	/* This is different from a potential 'can_change_position' or something which has a teleport event
	 * */
	bool can_move(
			const entt::registry& registry,
			const World::Grid& grid,
			const entt::entity entity,
			const World::GlobalPosition& to)
	{
		if (!registry.all_of<
				Game::Component::Value::Position,
				Game::Component::Resource::MovementPoints,
				Game::Component::Value::CollisionMovement
				>(entity))
			return false;

		const auto& current =
			registry.get<Game::Component::Value::Position>(entity).value;

		const auto move_distance = distance(current, to);

		if (blocks_movement(registry, grid, to) &&
				registry.get<Component::Value::CollisionMovement>(entity).value == true)
			return false;

		// TODO: invent movement cost
		if (move_distance >
				registry.get<Game::Component::Resource::MovementPoints>(entity).current)
			return false;

		// Can always move just one step at a time. Diagonal distance is ~1.4
		// Jumping is a different thing
		if (move_distance > 1.5)
			return false;

		// when moving diagonally, both corners have to be non-blocking
		if (move_distance > 1.1)
		{
			const auto corner1 = World::GlobalPosition{current.y, to.x};
			const auto corner2 = World::GlobalPosition{to.y, current.x};
			if (!can_move(registry, grid, entity, corner1)
					|| !can_move(registry, grid, entity, corner2))
				return false;
		}

		return true;
	}
}
