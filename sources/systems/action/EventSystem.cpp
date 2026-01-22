#include "components/Components.hpp"
#include "systems/items/LootSystem.hpp"
#include "systems/rendering/RenderingSystem.hpp"
#include "systems/rendering/LightingSystem.hpp"
#include "domain/Cell.hpp"
#include "domain/Event.hpp"
#include "external/entt/entt.hpp"
#include "systems/action/EventSystem.hpp"
#include "utils/ECS.hpp"
#include "infrastructure/GameLogger.hpp"

namespace EventSystem
{
	/* Transition component will transition to its destination
	 * DestroyOnContact will destroy when moving on same cell
	 * */
	void resolve_move_event(entt::registry& registry, const Event& event)
	{
		/* actor.position is optional,
		 * might be spawning with move(),
		 * which is good because then that will lead here.
		 *
		 * If there is actor.position, can check "leave cell" events
		 * */
		assert(event.actor.entity != entt::null);
		assert(event.target.position.is_valid());

		// Check entered cell
		const Position& entered = event.target.position;
		for (const auto entity : ECS::get_entities(registry, entered))
		{
			if (registry.all_of<Transition>(entity))
				TransitionSystem::transition(registry, event.actor.entity, entity);
			else if (registry.all_of<DestroyWhenStacked>(entity))
				ECS::destroy_entity(registry, entity);
		}
	}

	/* What are the consequences of gathering?
	 * */
	void resolve_gather_event(entt::registry& registry, const Event& event)
	{
		assert(event.actor.entity != entt::null);
		assert(event.actor.position.is_valid());
		assert(event.target.entity != entt::null);
		assert(event.target.position.is_valid());
		const auto& comp = registry.get<Gatherable>(event.target.entity);
		LootSystem::give_loot(registry, event.actor.entity, comp.loot_table_ids);
		if (comp.destroy == true)
			ECS::destroy_entity(registry, event.target.entity);
		else if (comp.lose_glow == true && registry.all_of<Glow>(event.target.entity))
		{
			registry.erase<Glow>(event.target.entity);
			LightingSystem::reset_lights(registry, event.target.position.cave_idx);
		}
		registry.erase<Gatherable>(event.target.entity);
	}

	/* Spawning is when entity gains a position.
	 * It can happen when it is first created into position,
	 * or when item is dropped from inventory, or otherwise gets Position emplaced
	 * */
	void resolve_spawn_event(entt::registry& registry, const Event& event)
	{
		assert(event.target.entity != entt::null && event.target.position.is_valid());
		if (registry.all_of<Glow, Position, FGColor>(event.target.entity))
			LightingSystem::reset_lights(registry, registry.get<Position>(event.target.entity).cave_idx);
	}

	/* Opposite of spawn event. Remove lights and other effects
	 * Entity is not yet destroyed, but will be after resolving all events
	 * */
	void resolve_destroy_event(entt::registry& registry, const Event& event)
	{
		if (registry.all_of<Glow, Position>(event.target.entity))
		{
			registry.erase<Glow, Position>(event.target.entity);
			LightingSystem::reset_lights(registry, registry.get<Position>(event.target.entity).cave_idx);
		}
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
			case Effect::Type::ReceiveItem:
				message += "receives ";
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
		auto& queue = registry.ctx().get<EventQueue>().queue;
		for (size_t i = 0; i < queue.size(); ++i)
		{
			const auto& event = queue[i];
			switch (event.effect.type)
			{
				case Effect::Type::Move:
					resolve_move_event(registry, event);
					break;
				case Effect::Type::Gather:
					resolve_gather_event(registry, event);
					break;
				case Effect::Type::Drop:
				case Effect::Type::Spawn:
					resolve_spawn_event(registry, event);
					break;
				case Effect::Type::Equip:
				case Effect::Type::Unequip:
					break;
				case Effect::Type::ReceiveItem:
					break;
				case Effect::Type::DestroyEntity:
					resolve_destroy_event(registry, event);
					break;
				default:
					Log::error("Unhandled effect type: " + std::to_string(static_cast<int>(event.effect.type)));
			}
			log_event(registry, event);
		}
		RenderingSystem::render(registry);
		queue.clear();

		// Destroy entities marked for destruction
		for (const auto entity : registry.view<Destroyed>())
			registry.destroy(entity);
	}
};
