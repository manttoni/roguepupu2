#include "entt.hpp"
#include "Cell.hpp"
#include "Cave.hpp"
#include "World.hpp"
#include "Components.hpp"
#include "ECS.hpp"
#include "Utils.hpp"
#include "systems/ConditionSystem.hpp"
#include "systems/EffectSystem.hpp"
#include "systems/TransitionSystem.hpp"
#include "Event.hpp"

namespace TriggerSystem
{
	void resolve_trigger(entt::registry& registry, Trigger trigger, const entt::entity owner, const entt::entity triggerer)
	{
		if (!ConditionSystem::is_true(registry, trigger.conditions, triggerer))
			return;

		Log::log("Trigger! " + registry.get<Name>(owner).name);

		switch (trigger.target.type)
		{
			case Target::Type::Self:
				trigger.target.entity = owner;
				break;
			case Target::Type::Actor:
				trigger.target.entity = triggerer;
				break;
			case Target::Type::Cell:
				trigger.target.cell = ECS::get_cell(registry, owner);
				break;
			default:
				Log::error("Unknown trigger target type: " + std::to_string(static_cast<size_t>(trigger.target.type)));
		}

		switch (trigger.effect.type)
		{
			case Effect::Type::Transition:
				break;
			default:
				break;
		}
		EffectSystem::resolve_effect(registry, trigger.effect, trigger.target);
	}
};
