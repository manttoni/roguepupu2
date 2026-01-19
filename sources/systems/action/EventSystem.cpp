#include "components/Components.hpp"
#include "systems/rendering/LightingSystem.hpp"
#include "domain/Cell.hpp"
#include "domain/Event.hpp"
#include "external/entt/entt.hpp"
#include "systems/action/EventSystem.hpp"
#include "systems/action/TriggerSystem.hpp"
#include "utils/ECS.hpp"
#include "infrastructure/GameLogger.hpp"

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

	void resolve_spawn_event(entt::registry& registry, const Event& event)
	{
		if (registry.all_of<Glow, Position, FGColor>(event.target.entity))
			LightingSystem::reset_lights(registry, registry.get<Position>(event.target.entity).cave_idx);
	}

	void log_event(entt::registry& registry, const Event& event)
	{
		std::string message;
		if (event.actor.entity != entt::null)
			message += ECS::get_colored_name(registry, event.actor.entity) + " ";
		switch (event.effect.type)
		{
			case Effect::Type::Gather:
				message += "gathers from ";
				break;
			case Effect::Type::Equip:
				message += "equips ";
				break;
			case Effect::Type::Unequip:
				message += "unequips ";
				break;
			case Effect::Type::Drop:
				message += "drops ";
				break;
			default:
				return;
		}
		if (event.target.entity != entt::null)
			message += ECS::get_colored_name(registry, event.target.entity);
		registry.ctx().get<GameLogger>().log(message);
	}

	void resolve_events(entt::registry& registry)
	{
		std::vector<Event>& event_queue = registry.ctx().get<EventQueue>().queue;
		for (auto& event : event_queue)
		{
			switch (event.effect.type)
			{
				case Effect::Type::Move:
					resolve_move_event(registry, event);
					break;
				case Effect::Type::Gather:
					resolve_gather_event(registry, event);
					break;
				case Effect::Type::Spawn:
					resolve_spawn_event(registry, event);
					break;
				case Effect::Type::Equip:
				case Effect::Type::Unequip:
					// if there will be equipment effects handle them here
					break;
				case Effect::Type::Drop:
					resolve_spawn_event(registry, event);
					break;
				default:
					Log::error("Unhandled effect type: " + std::to_string(static_cast<int>(event.effect.type)));
			}
			log_event(registry, event);
		}
		event_queue.clear();
	}
};
