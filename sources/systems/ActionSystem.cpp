#include "systems/ActionSystem.hpp"
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
#include "Intent.hpp"
#include "World.hpp"

namespace ActionSystem
{
	bool can_act(const entt::registry& registry, const entt::entity actor)
	{
		if (registry.all_of<Actions>(actor))
		{
			const auto& actions = registry.get<Actions>(actor);
			return actions.used < actions.total;
		}
		return false;
	}
	void use_action(entt::registry& registry, const entt::entity actor)
	{
		if (!registry.all_of<Actions>(actor))
			return;
		registry.get<Actions>(actor).used++;
	}
	void restore_actions(entt::registry& registry, const entt::entity actor)
	{
		if (!registry.all_of<Actions>(actor))
			return;
		registry.get<Actions>(actor).used = 0;
	}
	void resolve_events(entt::registry& registry, const entt::entity actor)
	{
		Cell* cell = ECS::get_cell(registry, actor);
		const auto& entities = cell->get_entities();
		for (const auto& e : entities)
		{
			if (actor == ECS::get_player(registry) &&
				registry.all_of<Transition>(e) &&
				registry.get<Transition>(e).destination != entt::null)
			{
				// Player stepped on a portal which is connected to another portal
				const auto destination = registry.get<Transition>(e).destination;
				if (!registry.all_of<Position>(destination) && registry.get<Name>(destination).name == "source")
				{
					// The destination doesn't have a position because the cave is not yet generated
					const size_t level = cell->get_cave()->get_level();
					cell->get_cave()->get_world()->get_cave(level + 1);
				}
				assert(registry.all_of<Position>(destination));
				MovementSystem::move(registry, actor, ECS::get_cell(registry, destination));
				break;
			}
		}
	}
	void resolve_intent(entt::registry& registry, const entt::entity actor, Intent intent)
	{
		switch (intent.type)
		{
			case Intent::Type::ExamineCell:
				ContextSystem::examine_cell(registry, intent.target_cell);
				break;
			case Intent::Type::OpenInventory:
				ContextSystem::show_entities_list(registry, intent.target);
				break;
			case Intent::Type::ShowPlayer:
				ContextSystem::show_entity_details(registry, actor);
				break;
			case Intent::Type::Move:
				MovementSystem::move(registry, actor, intent.target_cell);
				resolve_events(registry, actor);
				break;
			case Intent::Type::Attack:
				CombatSystem::attack(registry, actor, intent.target);
				break;
			case Intent::Type::Unlock:
				AccessSystem::unlock(registry, actor, intent.target);
				break;
			case Intent::Type::Open:
				AccessSystem::open(registry, actor, intent.target);
				break;
			case Intent::Type::UseAbility:
				registry.get<Abilities>(actor).abilities.at(intent.ability_id).use(registry, intent.target_cell);
				break;
			case Intent::Type::Hide:
				registry.emplace_or_replace<Hidden>(actor);
				break;
			case Intent::Type::DoNothing:
				use_action(registry, actor);
				return;
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
			if (AccessSystem::is_locked(registry, target))
				return {Intent::Type::Unlock, target_cell, target};
			if (AccessSystem::is_closed(registry, target))
				return {Intent::Type::Open, target_cell, target};
			if (FactionSystem::is_enemy(registry, actor, target))
				return {Intent::Type::Attack, target_cell, target};
		}
		if (MovementSystem::can_move(*cave, current_idx, target_idx))
			return {.type = Intent::Type::Move, .target_cell = target_cell};
		return {.type = Intent::Type::None};
	}

	Intent get_player_intent(entt::registry& registry)
	{
		const auto player = ECS::get_player(registry);
		Cave* cave = ECS::get_active_cave(registry);
		while (can_act(registry, player))
		{
			registry.ctx().get<Renderer>().render();

			int key = UI::instance().input(500);
			const Vec2 direction = UI::instance().get_direction(key);
			if (direction != Vec2{0, 0})
				return get_direction_intent(registry, player, direction);
			switch (key)
			{
				case KEY_RIGHT_CLICK:
					return {.type = Intent::Type::Attack, .target_cell = UI::instance().get_clicked_cell(*cave)};
				case KEY_LEFT_CLICK:
					return {.type = Intent::Type::ExamineCell, .target_cell = UI::instance().get_clicked_cell(*cave)};
				case 'i':
					return {.type = Intent::Type::OpenInventory, .target = player};
				case 'c':
					return {.type = Intent::Type::ShowPlayer};
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



	void player_turn(entt::registry& registry)
	{
		const auto player = ECS::get_player(registry);
		Intent intent = get_player_intent(registry);
		resolve_intent(registry, player, intent);
		restore_actions(registry, player);
	}

	void npc_turn(entt::registry& registry)
	{
		Cave* cave = ECS::get_active_cave(registry);
		for (const auto npc : cave->get_npcs())
		{
			Intent intent = AISystem::get_npc_intent(registry, npc);
			resolve_intent(registry, npc, intent);
			restore_actions(registry, npc);
		}
	}

	void environment_turn(entt::registry& registry)
	{
		(void) registry;
	}
};
