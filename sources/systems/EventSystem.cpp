#include "systems/EventSystem.hpp"
#include "systems/TriggerSystem.hpp"
#include "Components.hpp"
#include "Cell.hpp"
#include "entt.hpp"
#include "Event.hpp"

namespace EventSystem
{
	void resolve_move_event(entt::registry& registry, const Event& event)
	{
		// Check EnterCell triggers
		const auto entities = event.move_to->get_entities();
		for (const auto entity : entities)
		{
			if (!registry.all_of<Triggers>(entity) || entity == event.actor)
				continue;
			for (auto trigger : registry.get<Triggers>(entity).triggers)
			{
				if (trigger.type != Trigger::Type::EnterCell)
					continue;
				TriggerSystem::resolve_trigger(registry, trigger, entity, event.actor);
			}
		}

		// Check LeaveCell triggers etc...
	}

	void resolve_events(entt::registry& registry)
	{
		std::vector<Event>& event_queue = registry.ctx().get<EventQueue>().queue;
		for (auto& event : event_queue)
		{
			switch (event.type)
			{
				case Event::Type::Move:
					resolve_move_event(registry, event);
					break;
				default:
					break;
			}
		}
		event_queue.clear();
	}
};
