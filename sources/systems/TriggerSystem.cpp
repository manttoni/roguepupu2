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
		Log::log("Resolving trigger. Owner = " + registry.get<Name>(owner).name + " | Triggerer = " + registry.get<Name>(triggerer).name);
		if (!ConditionSystem::is_true(registry, trigger.conditions, triggerer))
			return;

		Log::log("Passed condition check");

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

		if (registry.all_of<Transition>(owner))
		{
			const auto destination = TransitionSystem::get_destination(registry, owner);
			if (!registry.all_of<Position>(destination) &&
					registry.get<Name>(destination).name == "source" &&
					registry.get<Name>(owner).name == "sink")
			{	// destination doesnt have a position because the cave is not yet generated
				Cell* cell = ECS::get_cell(registry, owner);
				Cave* cave = cell->get_cave();
				World* world = cave->get_world();
				const size_t level = cave->get_level();
				world->get_cave(level + 1);
			}
			assert(registry.all_of<Position>(destination));
			trigger.target.cell = ECS::get_cell(registry, destination);
		}

		EffectSystem::resolve_effect(registry, trigger.effect, trigger.target);
	}
};
