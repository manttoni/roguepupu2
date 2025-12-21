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
	void resolve_intent(entt::registry& registry, const entt::entity actor, Intent intent)
	{
		switch (intent.type)
		{
			case Type::ExamineCell:
				ContextSystem::examine_cell(registry, intent.target_cell);
				break;
			case Type::OpenInventory:
				ContextSystem::show_entities_list(registry, intent.target);
				break;
			case Type::ShowPlayer:
				ContextSystem::show_entity_details(registry, actor);
				break;
			case Type::Move:
				MovementSystem::move(registry, actor, intent.target_cell);
				// if (TrapSystem::is_trapped(registry, intent.target_cell))...
				break;
			case Type::Attack:
				CombatSystem::attack(registry, actor, intent.target);
				break;
			case Type::Transition:
				TransitionSystem::transition(registry, actor, intent.target);
				break;
			case Type::Unlock:
				AccessSystem::unlock(registry, actor, intent.target);
				break;
			case Type::Open:
				AccessSystem::open(registry, actor, intent.target);
				break;
			case Type::DoNothing:
				use_action(registry, actor);
				return;
			case Type::None:
			default:
				return;
		}
		Renderer::render(registry);
	}

	Intent get_direction_intent(const entt::registry& registry, const entt::entity actor, const Vec2 direction)
	{
		Cave* cave = ECS::get_active_cave(registry);
		Cell* current_cell = ECS::get_cell(registry, actor);
		const size_t current_idx = current_cell->get_idx();
		const size_t y = current_idx / cave->get_width() + direction.y;
		const size_t x = current_idx % cave->get_width() + direction.x;
		const size_t target_idx = y * cave->get_width() + x;
		Cell* target_cell = &cave->get_cell(target_idx);
		if (target_cell == nullptr)
			return {Type::None};
		const auto& entities = target_cell->get_entities();
		for (const auto target : entities)
		{
			if (AccessSystem::is_locked(registry, target))
				return {Type::Unlock, target_cell, target};
			if (AccessSystem::is_closed(registry, target))
				return {Type::Open, target_cell, target};
			if (FactionSystem::is_enemy(registry, actor, target))
				return {Type::Attack, target_cell, target};
			//if (InventorySystem::has_inventory(registry, target))
			//	return {Type::OpenInventory, target_cell, target};
			if (TransitionSystem::is_portal(registry, target))
				return {Type::Transition, target_cell, target};
		}
		if (MovementSystem::can_move(*cave, current_idx, target_idx))
			return {.type = Type::Move, .target_cell = target_cell};
		return {.type = Type::None};
	}

	Intent get_player_intent(entt::registry& registry)
	{
		const auto player = ECS::get_player(registry);
		Cave* cave = ECS::get_active_cave(registry);
		while (can_act(registry, player))
		{
			Renderer::render(registry);
			registry.ctx().get<GameState>().render_frame++;

			int key = UI::instance().input(500);
			const Vec2 direction = UI::instance().get_direction(key);
			if (direction != Vec2{0, 0})
				return get_direction_intent(registry, player, direction);
			switch (key)
			{
				case KEY_RIGHT_CLICK:
					return {.type = Type::Attack, .target_cell = UI::instance().get_clicked_cell(*cave)};
				case KEY_LEFT_CLICK:
					return {.type = Type::ExamineCell, .target_cell = UI::instance().get_clicked_cell(*cave)};
				case 'i':
					return {.type = Type::OpenInventory, .target = player};
				case 'c':
					return {.type = Type::ShowPlayer};
				case KEY_ESCAPE:
					registry.ctx().get<GameState>().running = false;
					[[fallthrough]];
				case ' ':
					return {.type = Type::DoNothing};
				default:
					break;
			}
		}
		return {.type = Type::None};
	}

	void player_turn(entt::registry& registry)
	{
		Log::log("Player turn");
		const auto player = ECS::get_player(registry);
		Intent intent = get_player_intent(registry);
		resolve_intent(registry, player, intent);
		restore_actions(registry, player);
	}

	void npc_turn(entt::registry& registry)
	{
		Log::log("npc turn");
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
