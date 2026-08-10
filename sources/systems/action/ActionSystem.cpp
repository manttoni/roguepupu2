#include <cassert>
#include <ncurses.h>
#include <vector>

#include "systems/action/InputSystem.hpp"
#include "UI/menus/SettingsMenu.hpp"
#include "UI/Dialog.hpp"
#include "UI/UI.hpp"
#include "components/Component.hpp"
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
#include "UI/menus/DevMenu.hpp"
#include "systems/state/AlignmentSystem.hpp"
#include "utils/ECS.hpp"
#include "utils/Error.hpp"
#include "domain/Action.hpp"
#include "domain/Position.hpp"
#include "external/entt/entity/fwd.hpp"
#include "utils/Debug.hpp"
#include "utils/Vec2.hpp"
#include "domain/Alignment.hpp"

/* ActionPoints, BonusActionPoints are components
 * Action is struct that answers questions like who does to who, with what and where
 * Action will mostly be possible but in case it's not, it will be aborted at any point
 * (For example stepping on a trap, or trying to move too far or otherwise having too few resources)
 * Actor is someone who can have ActionPoints
 * ActionSystem makes all Actors do something
 * */

using namespace Component::Resource;

namespace ActionSystem
{
	void end_turn(entt::registry& registry, const entt::entity entity)
	{
		assert((registry.all_of<ActionPoints, BonusActionPoints, MovementPoints>(entity)));
		registry.get<ActionPoints>(entity).current = 0;
		registry.get<BonusActionPoints>(entity).current = 0;
		registry.get<MovementPoints>(entity).current = 0.0;
	}
	void handle(entt::registry& registry, const NullAction& action)
	{
		(void) registry; (void) action;
	}
	void handle(entt::registry& registry, const MoveAction& action)
	{
		if (MovementSystem::can_move(registry, action.entity, action.destination))
			MovementSystem::move(registry, action.entity, action.destination);
	}
	void handle(entt::registry& registry, const AttackAction& action)
	{
		if (CombatSystem::can_attack(registry, action.attacker, action.defender))
			CombatSystem::attack(registry, action.attacker, action.defender);
	}
	void handle(entt::registry& registry, const EndTurnAction& action)
	{
		end_turn(registry, action.entity);
	}
	void handle(entt::registry& registry, const EquipAction& action)
	{
		if (EquipmentSystem::can_equip(registry, action.entity, action.item))
			EquipmentSystem::equip(registry, action.entity, action.item);
	}
	void handle(entt::registry& registry, const UnequipAction& action)
	{
		if (EquipmentSystem::can_unequip(registry, action.entity, action.item))
			EquipmentSystem::unequip(registry, action.entity, action.item);
	}
	void handle(entt::registry& registry, const EscapeAction& action)
	{
		(void) action;
		registry.ctx().get<GameState>().game_running = false;
	}
	void handle(entt::registry& registry, const UseAbilityAction& action)
	{
		(void) registry; (void) action; // Not implemented
	}
	void handle(entt::registry& registry, const CastSpellAction& action)
	{
		(void) registry; (void) action; // Not implemented
	}
	void resolve_actions(entt::registry& registry, const std::vector<Action>& actions)
	{
		for (const auto& action : actions)
		{
			std::visit(
					[&](const auto& a)
					{
					handle(registry, a);
					},
					action
					);
		}
	}

	bool can_act(const entt::registry& registry, const entt::entity actor)
	{
		return
			registry.get<Component::Resource::ActionPoints>(actor).current > 0 ||
			registry.get<Component::Resource::BonusActionPoints>(actor).current > 0 ||
			registry.get<Component::Resource::MovementPoints>(actor).current > 0.0;

	}

	void act_turn(entt::registry& registry, const entt::entity actor)
	{
		while (can_act(registry, actor) && ECS::game_running(registry))
		{
			std::vector<Action> actions;
			if (actor == ECS::get_player(registry))
				actions = InputSystem::get_actions(registry);
			else
				actions = AISystem::get_actions(registry, actor);

			resolve_actions(registry, actions);
			EventSystem::resolve_events(registry);
		}
	}

	void reset_action_points(entt::registry& registry, const entt::entity actor)
	{
		using namespace Component::Resource;
		// doesnt need ref?
		auto [action_points, bonus_points, movement_points] = registry.get<ActionPoints, BonusActionPoints, MovementPoints>(actor);
		action_points.current = action_points.maximum;
		bonus_points.current = bonus_points.maximum;
		movement_points.current = movement_points.maximum;
	}

	void act_round(entt::registry& registry, const size_t cave_idx)
	{
		auto actors = ECS::get_entities<Component::Tag::Actor>(registry, cave_idx);
		if (actors.empty())
		{
			Log::warning() << "act_round has no actors";
			return;
		}

		const auto player = ECS::get_player(registry); // optional, for example tests or simulations might not have this
		for (const auto actor : actors)
		{
			if (!registry.valid(actor) ||
					actor == entt::null ||
					registry.any_of<Component::Tag::Dead>(actor))
				continue;

			reset_action_points(registry, actor);
			act_turn(registry, actor);
			if (!ECS::game_running(registry))
				break;
			if (player != entt::null &&
					registry.get<Position>(player).cave_idx != cave_idx) // player left the cave
				break;
		}
	}

};
