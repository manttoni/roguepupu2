#include <cassert>
#include <ncurses.h>
#include <vector>

#include "ui/menus/DevMenu.hpp"
#include "components/Component.hpp"
#include "domain/Action.hpp"
#include "domain/Cave.hpp"
#include "domain/Position.hpp"
#include "external/entt/entity/fwd.hpp"
#include "external/entt/entt.hpp"
#include "infrastructure/GameState.hpp"
#include "systems/AI.hpp"
#include "systems/Action.hpp"
#include "systems/Combat.hpp"
#include "systems/Context.hpp"
#include "systems/Equipment.hpp"
#include "systems/Event.hpp"
#include "systems/Gathering.hpp"
#include "systems/Input.hpp"
#include "systems/Movement.hpp"
#include "rendering/Renderer.hpp"
#include "ui/Dialog.hpp"
#include "ui/menus/SettingsMenu.hpp"
#include "utils/Debug.hpp"
#include "utils/ECS.hpp"
#include "utils/Error.hpp"
#include "utils/Vec2.hpp"

/* ActionPoints, BonusActionPoints are components
 * Action is struct that answers questions like who does to who, with what and where
 * Action will mostly be possible but in case it's not, it will be aborted at any point
 * (For example stepping on a trap, or trying to move too far or otherwise having too few resources)
 * Actor is someone who can have ActionPoints
 * ActionSystem makes all Actors do something
 * */



namespace System::Action
{
	void end_turn(entt::registry& registry, const entt::entity entity)
	{
		assert((registry.all_of<Component::Resource::ActionPoints, Component::Resource::BonusActionPoints, Component::Resource::MovementPoints>(entity)));
		registry.get<Component::Resource::ActionPoints>(entity).current = 0;
		registry.get<Component::Resource::BonusActionPoints>(entity).current = 0;
		registry.get<Component::Resource::MovementPoints>(entity).current = 0.0;
	}
	void handle(entt::registry&, const Domain::Action::Null&)
	{
	}
	void handle(entt::registry& registry, const Domain::Action::Move& action)
	{
		if (System::Movement::can_move(registry, action.entity, action.destination))
			System::Movement::move(registry, action.entity, action.destination);
	}
	void handle(entt::registry& registry, const Domain::Action::Attack& action)
	{
		if (System::Combat::can_attack(registry, action.attacker, action.defender))
			System::Combat::attack(registry, action.attacker, action.defender);
	}
	void handle(entt::registry& registry, const Domain::Action::EndTurn& action)
	{
		end_turn(registry, action.entity);
	}
	void handle(entt::registry& registry, const Domain::Action::Equip& action)
	{
		if (System::Equipment::can_equip(registry, action.entity, action.item))
			System::Equipment::equip(registry, action.entity, action.item);
	}
	void handle(entt::registry& registry, const Domain::Action::Unequip& action)
	{
		if (System::Equipment::can_unequip(registry, action.entity, action.item))
			System::Equipment::unequip(registry, action.entity, action.item);
	}
	void handle(entt::registry& registry, const Domain::Action::QuitGame& action)
	{
		(void) action;
		registry.ctx().get<GameState>().game_running = false;
	}
	void handle(entt::registry& registry, const Domain::Action::UseAbility& action)
	{
		(void) registry; (void) action; // Not implemented
	}
	void handle(entt::registry& registry, const Domain::Action::CastSpell& action)
	{
		(void) registry; (void) action; // Not implemented
	}
	void resolve_actions(entt::registry& registry, const std::vector<Domain::Action::Any>& actions)
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
			std::vector<Domain::Action::Any> actions;
			if (actor == ECS::get_player(registry))
				actions = System::Input::get_actions(registry);
			else
				actions = System::AI::get_actions(registry, actor);

			resolve_actions(registry, actions);
			System::Event::resolve_events(registry);
			registry.ctx().get<Renderer>().render(registry);
		}
	}

	void reset_action_points(entt::registry& registry, const entt::entity actor)
	{
		// doesnt need ref?
		auto [action_points, bonus_points, movement_points] = registry.get<Component::Resource::ActionPoints, Component::Resource::BonusActionPoints, Component::Resource::MovementPoints>(actor);
		action_points.current = action_points.maximum;
		bonus_points.current = bonus_points.maximum;
		movement_points.current = movement_points.maximum;
	}

	void act_round(entt::registry& registry, const size_t cave_idx)
	{
		auto actors = ECS::get_entities<Component::Tag::Actor>(registry, cave_idx);
		if (actors.empty())
			Error::fatal("act_round has no actors");

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
					registry.get<Domain::Position>(player).cave_idx != cave_idx) // player left the cave
				break;
		}
	}

};
