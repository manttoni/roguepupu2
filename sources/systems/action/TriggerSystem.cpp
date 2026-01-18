#include "external/entt/entt.hpp"
#include "domain/Trigger.hpp"
#include "domain/Event.hpp"
#include "domain/Actor.hpp"
#include "domain/Effect.hpp"
#include "domain/Target.hpp"
#include "systems/position/TransitionSystem.hpp"
#include "systems/action/EffectSystem.hpp"
#include "systems/action/TriggerSystem.hpp"
#include "systems/action/ConditionSystem.hpp"

namespace TriggerSystem
{
	/* Some event has led to a trigger being triggered
	 * Trigger has Conditions that the triggerer mush match
	 * Trigger has Effect that will happen, leading to another Event
	 * */
	void resolve_trigger(entt::registry& registry, const Trigger& trigger, const Event& event)
	{
		// Check if actor can trigger this
		if (!ConditionSystem::is_true(registry, trigger.conditions, event.actor.entity))
			return;

		// Effect will now happen
		// Some data needs to be updated
		Actor actor = event.actor;
		Effect effect = trigger.effect;
		Target target = event.target;
		switch (effect.type)
		{
			case Effect::Type::Transition:
				// target is a passage. change it into its destination
				// get_destination_position will create the destination if it doesn't exist
				target.position = TransitionSystem::get_or_create_destination_position(registry, target.entity);
				target.entity = TransitionSystem::get_destination(registry, target.entity);
				break;
			default:
				break;
		}
		EffectSystem::resolve_effect(registry, actor, effect, target);
	}
};
