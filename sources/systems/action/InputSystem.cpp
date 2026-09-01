#include <vector>

#include "utils/Vec2.hpp"
#include "UI/UI.hpp"
#include "systems/action/InputSystem.hpp"
#include "systems/state/AttitudeSystem.hpp"
#include "domain/Action.hpp"
#include "external/entt/entt.hpp"
#include "utils/ECS.hpp"

/* This is for getting player input and turning it into game actions like movement, attacks...
 * Even if the action is impossible, it is no problem because execution will return back here after aborting it
 * */

namespace InputSystem
{
	std::vector<Action> bump_actions(const entt::registry& registry, const Position& position)
	{
		const auto player = ECS::get_player(registry);
		const auto& cave = ECS::get_cave(registry, position.cave_idx);
		if (cave.get_cell(position).get_type() == Cell::Type::Rock)
			return {};
		// todo: mining system
		// todo: climbing system?

		const auto target_entities = ECS::get_entities<Component::Value::CollisionMovement>(registry, position);
		assert(target_entities.size() < 2 && "not expecting more than one collisionmovement entity in one position");

		if (target_entities.empty())
		{
			return { MoveAction{
				.entity = player,
					.destination = position
			}};
		}

		const auto target_entity = target_entities.front();
		if (AttitudeSystem::is_enemy(registry, player, target_entity))
		{
			return { AttackAction{.attacker = player, .defender = target_entity}};
		}
		// todo: talking system
		// todo: opening doors
		return {};
	}

	/* Return a position the user has selected
	 * */
	Position get_target(const entt::registry& registry)
	{
		const auto player = ECS::get_player(registry);
		const auto& player_position = registry.get<Position>(player);
		const auto cave_idx = player_position.cave_idx;
		const auto& cave = ECS::get_cave(registry, cave_idx);
		const auto cave_size = cave.get_size();

		Vec2<int> target = Vec2<int>::from_idx(player_position.cell_idx, cave_size);
		while (true)
		{
			const int key = UI::instance().input(500);
			switch (key)
			{
				case '\n':
					return Position(target.to_idx(cave_size), cave_idx);
				case KEY_ESCAPE:
					return Position::invalid_position();
				default:
					{
						const auto direction = UI::instance().get_direction(key); // Returns { 0, 0 } if its not directional key
						const auto next = target + direction;
						if (direction != Vec2<int>{}
								&& cave.contains(next)
								&& VisionSystem::has_vision(
									registry,
									player,
									Position(next.to_idx(cave_size), cave_idx)))
						{	// Limiting to visible positions is probably not strictly necessary
							target = next;
						}
					}
					break;
			}
		}
	}

	std::vector<Action> get_actions(const entt::registry& registry)
	{
		const auto player = ECS::get_player(registry);
		const auto& player_pos = registry.get<Position>(player);
		const auto& cave = ECS::get_cave(registry, player_pos);
		while (true)
		{
			const auto key = UI::instance().input(500); // wait 500ms for input
			switch (key)
			{
				case 'c':
					break;
				case 'i':
					break;
				case '\n':
					{
						const auto target = get_target(registry);
						if (target.is_valid())
							break; //return ContextSystem::get_actions(registry, target);
						break;
					}
				case ' ':
					return { EndTurnAction{.entity = ECS::get_player(registry)} };
				case KEY_ESCAPE:
					return { EscapeAction{} };
				default:
					{
						const auto direction = UI::instance().get_direction(key);
						const auto offset_position = cave.offset_position(player_pos, direction);
						if (direction == Vec2<int>{} || !offset_position.is_valid())
							break;
						return bump_actions(registry, offset_position);
					}
			}
		}
		return {};
	}

}
