#include "systems/ActionSystem.hpp"
#include "systems/GatheringSystem.hpp"
#include "systems/EventSystem.hpp"
#include "systems/AbilitySystem.hpp"
#include "systems/AISystem.hpp"
#include "systems/ContextSystem.hpp"
#include "systems/MovementSystem.hpp"
#include "systems/CombatSystem.hpp"
#include "systems/TransitionSystem.hpp"
#include "systems/AccessSystem.hpp"
#include "systems/FactionSystem.hpp"
#include "systems/InventorySystem.hpp"
#include "UI.hpp"
#include "Cave.hpp"
#include "Cell.hpp"
#include "entt.hpp"
#include "GameState.hpp"
#include "ECS.hpp"
#include "Renderer.hpp"
#include "Components.hpp"
#include "Event.hpp"
#include "World.hpp"
#include "DevTools.hpp"

namespace ActionSystem
{
	void resolve_intent(entt::registry& registry, Intent intent)
	{
		Log::log(registry.get<Name>(intent.actor).name + " intent: ");
		switch (intent.type)
		{
			case Intent::Type::ExamineCell:
				ContextSystem::examine_cell(registry, intent.target.cell);
				break;
			case Intent::Type::OpenInventory:
				ContextSystem::show_entities_list(registry, intent.target.entity);
				break;
			case Intent::Type::ShowPlayer:
				ContextSystem::show_entity_details(registry, intent.actor);
				break;
			case Intent::Type::Move:
				Log::log("Move");
				MovementSystem::move(registry, intent.actor, intent.target.cell);
				break;
			case Intent::Type::Attack:
				CombatSystem::attack(registry, intent.actor, intent.target.entity);
				break;
			case Intent::Type::Unlock:
				AccessSystem::unlock(registry, intent.actor, intent.target.entity);
				break;
			case Intent::Type::Open:
				AccessSystem::open(registry, intent.actor, intent.target.entity);
				break;
			case Intent::Type::UseAbility:
				AbilitySystem::use_ability(registry, intent.actor, *intent.ability, intent.target);
				break;
			case Intent::Type::Gather:
				Log::log("Gather");
				GatheringSystem::gather(registry, intent.actor, intent.target.entity);
				break;
			case Intent::Type::Hide:
				if (registry.all_of<Hidden>(intent.actor))
					registry.erase<Hidden>(intent.actor);
				else
					registry.emplace<Hidden>(intent.actor);
				break;
			case Intent::Type::DoNothing:
			case Intent::Type::None:
			default:
				return;
		}
		registry.ctx().get<Renderer>().render();
	}

	Intent get_direction_intent(const entt::registry& registry, const entt::entity actor, const Vec2 direction)
	{
		Cave* cave = ECS::get_active_cave(registry);
		Cell* current_cell = ECS::get_cell(registry, actor);
		const size_t current_idx = current_cell->get_idx();
		const size_t y = current_idx / cave->get_width() + direction.y;
		const size_t x = current_idx % cave->get_width() + direction.x;
		const size_t target_idx = y * cave->get_width() + x;
		if (target_idx >= cave->get_cells().size())
			return {.type = Intent::Type::None};
		Cell* target_cell = &cave->get_cell(target_idx);
		if (target_cell == nullptr)
			return {.type = Intent::Type::None};
		const auto& entities = target_cell->get_entities();
		for (const auto target : entities)
		{
			if (FactionSystem::is_enemy(registry, actor, target))
			{
				Intent intent = {.type = Intent::Type::Attack};
				intent.target.entity = target;
				return intent;
			}
		}
		if (MovementSystem::can_move(*cave, current_idx, target_idx))
		{
			Intent intent = {.type = Intent::Type::Move};
			intent.target.cell = target_cell;
			return intent;
		}
		return {.type = Intent::Type::None};
	}

	Intent get_player_intent(entt::registry& registry)
	{
		const auto player = ECS::get_player(registry);
		Cave* cave = ECS::get_active_cave(registry);
		while (true)
		{
			registry.ctx().get<Renderer>().render();

			int key = UI::instance().input(500);
			if (key == '`') DevTools::dev_menu(registry);
			const Vec2 direction = UI::instance().get_direction(key);
			if (direction != Vec2{0, 0})
				return get_direction_intent(registry, player, direction);
			switch (key)
			{
				case KEY_RIGHT_CLICK:
					{
					Intent intent = {.type = Intent::Type::Attack};
					intent.target.cell = UI::instance().get_clicked_cell(*cave);
					return intent;
					}
				case KEY_LEFT_CLICK:
					{
					Intent intent = {.type = Intent::Type::ExamineCell};
					intent.target.cell = UI::instance().get_clicked_cell(*cave);
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
					[[fallthrough]];
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
		Cave* cave = ECS::get_active_cave(registry);
		const auto player = ECS::get_player(registry);
		std::vector<entt::entity> actors = cave->get_npcs();
		actors.push_back(player);
		for (const auto actor : actors)
		{
			if (!registry.valid(actor)) continue;
			Intent intent = actor == player ?
				get_player_intent(registry) :
				AISystem::get_npc_intent(registry, actor);
			intent.actor = actor;
			resolve_intent(registry, intent);
			EventSystem::resolve_events(registry);
			if (cave != ECS::get_active_cave(registry))
				return;
		}
	}

};
