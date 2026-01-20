#include "UI/UI.hpp"
#include "components/Components.hpp"
#include "domain/Cave.hpp"
#include "domain/Cell.hpp"
#include "domain/Event.hpp"
#include "domain/World.hpp"
#include "external/entt/entt.hpp"
#include "infrastructure/GameState.hpp"
#include "systems/action/AISystem.hpp"
#include "systems/action/AbilitySystem.hpp"
#include "systems/action/ActionSystem.hpp"
#include "systems/action/EventSystem.hpp"
#include "systems/combat/CombatSystem.hpp"
#include "systems/crafting/GatheringSystem.hpp"
#include "systems/position/MovementSystem.hpp"
#include "systems/position/TransitionSystem.hpp"
#include "systems/rendering/RenderingSystem.hpp"
#include "systems/state/ContextSystem.hpp"
#include "systems/state/InventorySystem.hpp"
#include "testing/DevTools.hpp"
#include "systems/state/AlignmentSystem.hpp"
#include "utils/ECS.hpp"

namespace ActionSystem
{
	// At this point the intent has been accepted and will be acted/executed
	void resolve_intent(entt::registry& registry, Intent intent)
	{
		switch (intent.type)
		{
			case Intent::Type::ExamineCell:
				ContextSystem::examine_cell(registry, intent.target.position);
				break;
			case Intent::Type::OpenInventory:
				ContextSystem::show_entities_list(registry, intent.target.entity);
				break;
			case Intent::Type::ShowPlayer:
				ContextSystem::show_entity_details(registry, ECS::get_player(registry));
				break;
			case Intent::Type::Move:
				MovementSystem::move(registry, intent.actor.entity, intent.target.position);
				break;
			case Intent::Type::Attack:
				CombatSystem::attack(registry, intent.actor.entity, intent.target.entity);
				break;
			case Intent::Type::UseAbility:
				AbilitySystem::use_ability(registry, intent.actor, intent.ability_id, intent.target);
				break;
			case Intent::Type::Gather:
				GatheringSystem::gather(registry, intent.actor.entity, intent.target.entity);
				break;
			case Intent::Type::Hide:
				if (registry.all_of<Hidden>(intent.actor.entity))
					registry.erase<Hidden>(intent.actor.entity);
				else
					registry.emplace<Hidden>(intent.actor.entity);
				break;
			case Intent::Type::DoNothing:
			case Intent::Type::None:
			default:
				return;
		}
		RenderingSystem::render(registry);
	}

	/* Figure out what actor.entity wants to do with direction
	 * If there is some interactable entity like an enemy, attack instead of moving.
	 * If there is just an empty cell, intent to move is very likely.
	 * */
	Intent get_direction_intent(const entt::registry& registry, const Actor& actor, const Vec2 direction)
	{
		assert(actor.entity != entt::null && actor.position.is_valid());
		const auto& cave = ECS::get_cave(registry, actor.position);
		const auto cave_size = cave.get_size();

		const Vec2 current(actor.position.cell_idx, cave_size);
		const Vec2 destination = current + direction;

		if (destination.out_of_bounds(0, cave_size - 1))
			Log::error("Destination position out of bounds");

		const Position destination_pos(destination.to_idx(cave_size), cave.get_idx());
		Intent intent;
		intent.actor = actor;
		intent.target.position = destination_pos;
		if (MovementSystem::can_move(registry, actor.position, destination_pos))
			intent.type = Intent::Type::Move;

		/* Check attack and other interactions */
		for (const auto entity : ECS::get_entities(registry, destination_pos))
		{
			if (intent.type != Intent::Type::None) break;
			if (AlignmentSystem::is_hostile(registry, actor.entity, entity))
			{	// actor.entity wants to attack entity
				intent.type = Intent::Type::Attack;
				intent.target.entity = entity;
			}
		}

		return intent;
	}

	Intent get_player_intent(entt::registry& registry)
	{
		const auto player = ECS::get_player(registry);
		const Actor player_actor = {
			.entity = player,
			.position = registry.get<Position>(player)
		}; // Making very explicit who is acting and where
		while (true)
		{
			RenderingSystem::render(registry);

			const int key = UI::instance().input(500);
			if (key == '`')
			{
				DevTools::dev_menu(registry);
				continue;
			}
			const Vec2 direction = UI::instance().get_direction(key);
			if (direction != Vec2{0, 0})
				return get_direction_intent(registry, player_actor, direction);
			switch (key)
			{
				case KEY_RIGHT_CLICK: // make this work when combat system is implemented
					{
					Intent intent = {.type = Intent::Type::Attack};
					intent.target.position = UI::instance().get_clicked_position(registry);
					return intent;
					}
				case KEY_LEFT_CLICK:
					{
					Intent intent = {.type = Intent::Type::ExamineCell};
					intent.target.position = UI::instance().get_clicked_position(registry);
					return intent;
					}
				case 'i':
					{
					Intent intent = {.type = Intent::Type::OpenInventory};
					intent.target.entity = player;
					return intent;
					}
				case 'c':
					return {.type = Intent::Type::ShowPlayer};
				case 'h':
					return {.type = Intent::Type::Hide};
				case KEY_ESCAPE:
					registry.ctx().get<GameState>().running = false;
					return {.type = Intent::Type::DoNothing};
				case ' ':
					return {.type = Intent::Type::DoNothing};
				default:
					break;
			}
		}
		return {.type = Intent::Type::None};
	}

	void act_round(entt::registry& registry)
	{
		const auto player = ECS::get_player(registry);
		const auto& player_pos = registry.get<Position>(player);
		const auto cave_idx = player_pos.cave_idx;

		std::vector<entt::entity> entities = ECS::get_entities_in_cave(
				registry,
				cave_idx,
				Category("creatures")
				);

		for (const auto entity : entities)
		{
			// entity might have been somehow disabled by some Event
			// Probably should not happen, but check anyway
			if (!registry.valid(entity)) continue;

			// Get entity intent. Intent is what they want to do.
			Intent intent = entity == player ?
				get_player_intent(registry) :
				AISystem::get_npc_intent(registry, entity);

			// This information has to be valid
			intent.actor.entity = entity;
			intent.actor.position = registry.get<Position>(entity);

			// Intent has been validated and will not be executed
			resolve_intent(registry, intent);

			// Events are things that happened.
			// They can have consequences,
			// which will be resolved by EventSystem
			EventSystem::resolve_events(registry);

			// If player leaves cave, end round
			if (cave_idx != registry.get<Position>(player).cave_idx)
				return;
		}
	}

};
