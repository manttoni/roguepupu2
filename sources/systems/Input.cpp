#include <vector>

#include "utils/Vec2.hpp"
#include "systems/Input.hpp"
#include "systems/Attitude.hpp"
#include "domain/Action.hpp"
#include "external/entt/entt.hpp"
#include "utils/ECS.hpp"
#include "ncurses/Input.hpp"

/* This is for getting player input and turning it into game actions like movement, attacks...
 * Even if the action is impossible, it is no problem because execution will return back here after aborting it
 * */


namespace System::Input

{
	std::vector<Domain::Action::Any> bump_actions(const entt::registry& registry, const Domain::Position& position)
	{
		const auto player = ECS::get_player(registry);
		const auto& cave = ECS::get_cave(registry, position.cave_idx);
		if (cave.get_cell(position).get_type() == Domain::Cell::Type::Rock)
			return {};
		// todo: mining system
		// todo: climbing system?

		const auto target_entities = ECS::get_entities<Component::Value::CollisionMovement>(registry, position);
		assert(target_entities.size() < 2 && "not expecting more than one collisionmovement entity in one position");

		if (target_entities.empty())
		{
			return { Domain::Action::Move{
				.entity = player,
					.destination = position
			}};
		}

		const auto target_entity = target_entities.front();
		if (System::Attitude::is_enemy(registry, player, target_entity))
		{
			return { Domain::Action::Attack{.attacker = player, .defender = target_entity}};
		}
		// todo: talking system
		// todo: opening doors
		return {};
	}

	/* Return a position the user has selected
	 * */
	Domain::Position get_target(const entt::registry& registry)
	{
		const auto player = ECS::get_player(registry);
		const auto& player_position = registry.get<Domain::Position>(player);
		const auto cave_idx = player_position.cave_idx;
		const auto& cave = ECS::get_cave(registry, cave_idx);
		const auto cave_size = cave.get_size();

		Vec2<int> target = Vec2<int>::from_idx(player_position.cell_idx, cave_size);
		while (true)
		{
			using Key = Ncurses::Input::Key;

			const auto event = Ncurses::Input::get_event(500);
			switch (event.key)
			{
				case Key::Enter:
					return Domain::Position(target.to_idx(cave_size), cave_idx);
				case Key::Escape:
					return Domain::Position::invalid();
				default:
					if (Ncurses::Input::is_directional(event.key))
					{
						const auto direction = Ncurses::Input::to_direction(event.key);
						const auto next = target + direction;
						if (cave.contains(next))
							target = next;
					}
					break;
			}
		}
	}

	std::vector<Domain::Action::Any> get_actions(const entt::registry& registry)
	{
		const auto player = ECS::get_player(registry);
		const auto& player_pos = registry.get<Domain::Position>(player);
		const auto& cave = ECS::get_cave(registry, player_pos);
		while (true)
		{
			const auto event = Ncurses::Input::get_event(500);
			using Key = Ncurses::Input::Key;
			switch (event.key)
			{
				case Key::Enter:
					{
						const auto target = get_target(registry);
						if (target.is_valid())
							break; //return System::Context::get_actions(registry, target);
						break;
					}
				case Key::Space:
					return { Domain::Action::EndTurn{.entity = ECS::get_player(registry)} };
				case Key::Escape:
					return { Domain::Action::QuitGame{} };
				default:
					if (Ncurses::Input::is_directional(event.key))
					{
						const auto direction = Ncurses::Input::to_direction(event.key);
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
