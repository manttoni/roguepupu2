#include <cassert>
#include <ncurses.h>
#include <vector>

#include "UI/menus/SettingsMenu.hpp"
#include "UI/Dialog.hpp"
#include "UI/UI.hpp"
#include "components/Components.hpp"
#include "domain/Cave.hpp"
#include "external/entt/entt.hpp"
#include "infrastructure/GameState.hpp"
#include "systems/action/AISystem.hpp"
#include "systems/action/AbilitySystem.hpp"
#include "systems/action/ActionSystem.hpp"
#include "systems/action/EventSystem.hpp"
#include "systems/combat/CombatSystem.hpp"
#include "systems/crafting/GatheringSystem.hpp"
#include "systems/position/MovementSystem.hpp"
#include "systems/rendering/RenderingSystem.hpp"
#include "systems/state/ContextSystem.hpp"
#include "systems/state/EquipmentSystem.hpp"
#include "infrastructure/DevTools.hpp"
#include "systems/state/AlignmentSystem.hpp"
#include "utils/ECS.hpp"
#include "utils/Error.hpp"
#include "domain/Actor.hpp"
#include "domain/Intent.hpp"
#include "domain/Position.hpp"
#include "domain/Target.hpp"
#include "external/entt/entity/fwd.hpp"
#include "systems/rendering/RenderData.hpp"
#include "utils/Vec2.hpp"

namespace ActionSystem
{
	// At this point the intent has been accepted and will be acted/executed
	void resolve_intent(entt::registry& registry, const Intent& intent)
	{
		switch (intent.type)
		{
			case Intent::Type::ExamineCell:
				ContextSystem::examine_cell(registry, intent.target.position);
				break;
			case Intent::Type::OpenInventory:
				ContextSystem::open_inventory(registry, intent.target.entity);
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
			case Intent::Type::Equip:
				EquipmentSystem::equip(registry, intent.actor.entity, intent.target.entity);
				break;
			case Intent::Type::SwapLoadout:
				EquipmentSystem::swap_loadout(registry, intent.actor.entity);
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
	 *
	 * This is used only for player
	 * */
	Intent get_direction_intent(const entt::registry& registry, const Vec2 direction)
	{
		const auto player = ECS::get_player(registry);
		Intent intent;
		intent.actor = {.entity = player, .position = registry.get<Position>(player)};
		const auto& cave = ECS::get_cave(registry, intent.actor.position);
		const auto cave_size = cave.get_size();

		const Vec2 current(intent.actor.position.cell_idx, cave_size);
		const Vec2 destination = current + direction;

		if (destination.out_of_bounds(0, cave_size - 1))
			Error::fatal("Destination position out of bounds");

		const Position destination_pos(destination.to_idx(cave_size), cave.get_idx());
		intent.target.position = destination_pos;
		if (MovementSystem::can_move(registry, intent.actor.position, destination_pos))
			intent.type = Intent::Type::Move;

		for (const auto entity : ECS::get_entities(registry, destination_pos))
		{
			if (intent.type != Intent::Type::None) break;
			if (registry.all_of<Alignment, Health>(entity) &&
				!registry.all_of<Dead>(entity) &&
				AlignmentSystem::is_hostile(registry, player, entity) &&
				AlignmentSystem::is_hostile(registry, entity, player))
			{
				intent.type = Intent::Type::Attack;
				intent.target.entity = entity;
				break;
			}
		}

		return intent;
	}

	Intent get_player_intent(entt::registry& registry)
	{
		const auto player = ECS::get_player(registry);
		const auto position = registry.get<Position>(player);
		while (true)
		{
			Intent intent;
			intent.actor.entity = player;
			intent.actor.position = position;

			RenderingSystem::render(registry);

			const int key = UI::instance().input(500);
			if (key == '`')
			{
				DevTools::dev_menu(registry);
				continue;
			}
			const Vec2 direction = UI::instance().get_direction(key);
			if (direction != Vec2{0, 0})
				return get_direction_intent(registry, direction);
			switch (key)
			{
				case KEY_RIGHT_CLICK: // make this work when combat system is implemented
					{
						intent.type = Intent::Type::Attack;
						intent.target.position = UI::instance().get_clicked_position(registry);
						if (!intent.target.position.is_valid() ||
								!ECS::get_attack_range(registry, player)
								.contains(ECS::distance(registry, intent.actor.position, intent.target.position)))
							continue;
						for (const auto entity : ECS::get_entities(registry, intent.target.position))
						{
							if (registry.all_of<Alignment>(entity) &&
									AlignmentSystem::is_hostile(registry, player, entity) &&
									AlignmentSystem::is_hostile(registry, entity, player))
								intent.target.entity = entity;
						}
						if (intent.target.entity == entt::null)
							continue;
						return intent;
					}
				case KEY_LEFT_CLICK:
					{
						Intent intent = {.type = Intent::Type::ExamineCell};
						intent.target.position = UI::instance().get_clicked_position(registry);
						if (!intent.target.position.is_valid())
							continue;
						return intent;
					}
				case '\n':
				case KEY_ENTER:
					{
						Intent intent = {.type = Intent::Type::ExamineCell};
						intent.target.position = UI::instance().get_selected_position(registry);
						if (!intent.target.position.is_valid())
							continue;
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
				case KEY_ESCAPE:
					{
						Menu::Element selection;
						selection = Dialog::get_selection("", {"Continue", "Controls", "Settings", "Quit"}, Screen::middle(), selection.index);
						//if (selection.label == "Controls")
						//	ControlsMenu::show_menu(registry);
						if (selection.label == "Settings")
							SettingsMenu::show_menu(registry);
						if (selection.label != "Quit")
							continue;
					}
					registry.ctx().get<GameState>().game_running = false;
					return {.type = Intent::Type::DoNothing};
				case ' ':
					return {.type = Intent::Type::DoNothing};
				case 'l':
					registry.ctx().get<RenderData>().print_log ^= true;
					continue;
				case 'w':
					return {.type = Intent::Type::SwapLoadout};
				default:
					break;
			}
		}
		return {.type = Intent::Type::None};
	}

	void act_round(entt::registry& registry, const size_t cave_idx)
	{
		std::vector<entt::entity> entities = ECS::get_entities_in_cave(
				registry,
				cave_idx,
				Category("creatures")
				);
		const auto player = ECS::get_player(registry); // if no player, this is entt::null
		for (const auto entity : entities)
		{
			if (!registry.valid(entity) || registry.any_of<Dead>(entity))
				continue;

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

			if (player != entt::null &&
					cave_idx != registry.get<Position>(player).cave_idx)
				return;
			if (registry.ctx().get<GameState>().game_running == false)
				break;
		}
	}

};
