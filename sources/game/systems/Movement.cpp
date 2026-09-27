#include "game/systems/Movement.hpp"
#include "game/entities/Entity.hpp"
#include "utils/Vec2.hpp"
#include "game/world/Position.hpp"

namespace
{
	// TODO: Extract shared spatial queries when needed.
	bool blocks_movement(
			const Game::Simulation& simulation,
			const Game::World::GlobalPosition& position)
	{
		const auto& world = simulation.get_world();
		const auto& registry = simulation.get_registry();
		const auto* cell = world.find_cell(position);

		if (!cell || cell->terrain == Game::Enum::Terrain::Rock)
			return true;

		const auto entities = Game::Entity::find_all(
				registry,
				Component::Value::Position{position},
				Component::Value::CollisionMovement{true}
				);

		return !entities.empty();
	}

	// GlobalPosition is alias for Vec2<int>
	double distance(
			const Game::World::GlobalPosition& from,
			const Game::World::GlobalPosition& to)
	{
		const auto& diff = from - to;
		return diff.length();
	}
}

namespace Game::System::Movement
{
	/* This is different from a potential 'can_change_position' or something which has a teleport event
	 * */
	bool can_move(const Simulation& simulation, const entt::entity entity, const World::GlobalPosition& to)
	{
		const auto& registry = simulation.get_registry();
		const auto& current = registry.get<Component::Value::Position>(entity).value;
		const auto move_distance = distance(current, to);

		if (!registry.all_of<Component::Value::Position>(entity))
			return false;

		if (blocks_movement(simulation, to))
			return false;

		// TODO: invent movement cost
		if (move_distance > registry.get<Component::Resource::MovementPoints>(entity).current)
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
			if (blocks_movement(simulation, corner1)
				|| blocks_movement(simulation, corner2))
				return false;
		}

		return true;
	}
}
