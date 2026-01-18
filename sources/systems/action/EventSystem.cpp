#include "components/Components.hpp"
#include "domain/Cell.hpp"
#include "domain/Event.hpp"
#include "external/entt/entt.hpp"
#include "systems/action/EventSystem.hpp"
#include "systems/action/TriggerSystem.hpp"
#include "utils/ECS.hpp"

namespace EventSystem
{
	/* Copy event because it needs more data
	 * */
	void resolve_move_event(entt::registry& registry, Event event)
	{
		const auto entities = ECS::get_entities(registry, event.target.position);
		for (const auto entity : entities)
		{
			if (!registry.all_of<Triggers>(entity) || entity == event.actor.entity)
				continue;
			for (auto trigger : registry.get<Triggers>(entity).triggers)
			{
				if (trigger.type != Trigger::Type::EnterCell)
					continue;
				// Have to add entity to event.target
				event.target.entity = entity;
				assert(event.actor.entity != entt::null);
				assert(event.actor.position.is_valid());
				assert(event.target.entity != entt::null);
				assert(event.target.position.is_valid());
				TriggerSystem::resolve_trigger(
						registry,
						trigger,	// trigger that has been triggered
						event		// event that led to the triggering
						);
			}
		}
	}

	void resolve_gather_event(entt::registry& registry, const Event& event)
	{
		if (!registry.all_of<Triggers>(event.target.entity))
			return;
		for (auto trigger : registry.get<Triggers>(event.target.entity).triggers)
			TriggerSystem::resolve_trigger(registry, trigger, event);
	}

	void resolve_events(entt::registry& registry)
	{
		std::vector<Event>& event_queue = registry.ctx().get<EventQueue>().queue;
		for (auto& event : event_queue)
		{
			switch (event.effect.type)
			{
				case Effect::Type::Move:
					// If there will be separate LeaveCell and EnterCell, split this before
					resolve_move_event(registry, event);
					break;
				case Effect::Type::Gather:
					resolve_gather_event(registry, event);
					break;
				default:
					Log::error("Unhandled effect type in resolve_events");
			}
		}
		event_queue.clear();
	}
};
