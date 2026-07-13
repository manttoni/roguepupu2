#include <assert.h>
#include <curses.h>
#include <stddef.h>
#include <optional>
#include <string>
#include <vector>
#include <iostream>

#include "components/Components.hpp"
#include "domain/Actor.hpp"
#include "domain/Damage.hpp"
#include "domain/Effect.hpp"
#include "domain/Event.hpp"
#include "infrastructure/EventQueue.hpp"
#include "domain/Position.hpp"
#include "domain/Target.hpp"
#include "external/entt/entity/fwd.hpp"
#include "external/entt/entt.hpp"
#include "infrastructure/EventLogger.hpp"
#include "systems/action/EventSystem.hpp"
#include "systems/items/LootSystem.hpp"
#include "systems/position/TransitionSystem.hpp"
#include "systems/rendering/LightingSystem.hpp"
#include "systems/rendering/RenderingSystem.hpp"
#include "systems/state/AlignmentSystem.hpp"
#include "systems/state/EquipmentSystem.hpp"
#include "utils/ECS.hpp"
#include "utils/Error.hpp"
#include "utils/Log.hpp"

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
		 * If there is actor.position, can check "leave cell" events,
		 * but that would conflict with finalize() in this file
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
	 *
	 * Will probably remake after loot_tables are made.
	 * There will be a gathering loot_table, and just roll from that
	 * */
	void resolve_gather_event(entt::registry& registry, const Event& event)
	{
		assert(event.actor.entity != entt::null);
		assert(event.actor.position.is_valid());
		assert(event.target.entity != entt::null);
		assert(event.target.position.is_valid());

		const auto gatherable = event.target.entity;
		const auto gather_effect = registry.get<GatherEffect>(gatherable);
		const auto loot_table = registry.get<LootTableRef>(gatherable);
		LootSystem::give_loot(registry, event.actor.entity, loot_table.id);
		switch (gather_effect)
		{
			case GatherEffect::None:
				break;
			case GatherEffect::Dim:
				registry.emplace_or_replace<NcursesAttr>(event.target.entity, A_DIM);
				if (registry.all_of<Glow>(event.target.entity))
				{
					registry.erase<Glow>(event.target.entity);
					LightingSystem::reset_lights(registry, event.target.position.cave_idx);
				}
				break;
			case GatherEffect::Destroy:
				ECS::destroy_entity(registry, event.target.entity);
				break;
		}
		registry.erase<Gatherable, GatherEffect, LootTableRef, RequiresTool>(event.target.entity);
	}

	/* Spawning is when entity gains a position.
	 * It can happen when it is first created into position,
	 * or when item is dropped from inventory, or otherwise gets Position emplaced
	 * */
	void resolve_spawn_event(entt::registry& registry, const Event& event)
	{
		assert(event.target.entity != entt::null && event.target.position.is_valid());
		if (registry.all_of<Glow, Position, Color>(event.target.entity))
			LightingSystem::reset_lights(registry, registry.get<Position>(event.target.entity).cave_idx);
	}

	/*void resolve_equip_event(entt::registry& registry, const Event& event)
	{

	}

	void resolve_unequip_event(entt::registry& registry, const Event& event)
	{

	}*/


	/* Opposite of spawn event. Remove lights and other effects
	 * Entity is not yet destroyed, but will be after resolving all events
	 * */
	void resolve_destroy_event(entt::registry& registry, const Event& event)
	{
		assert(event.target.entity != entt::null);
		assert(registry.valid(event.target.entity));
		if (registry.all_of<Glow, Position>(event.target.entity))
		{
			registry.erase<Glow>(event.target.entity);
			LightingSystem::reset_lights(registry, registry.get<Position>(event.target.entity).cave_idx);
		}
	}

	/* Health has already been lost.
	 * Damaged entity is the actor of event. There is no target.
	 * */
	void resolve_take_damage_event(entt::registry& registry, const Event& event)
	{
		(void) registry; (void) event;
	}

	/* If entity is npc, make them equip items when they receive them
	 * */
	void resolve_receive_item_event(entt::registry& registry, const Event& event)
	{
		const auto entity = event.actor.entity;
		const auto item = event.target.entity;
		assert(entity != entt::null && item != entt::null);

		if (entity != ECS::get_player(registry))
		{
			if (!registry.all_of<EquipmentSlots>(entity) || !registry.all_of<EquipmentSlot>(item))
				return;
			EquipmentSystem::equip_in_free_slots(registry, entity, item);
		}
	}

	/* If actor attacked non-hostile targets make them grumpy
	 * */
	void resolve_attack_event(entt::registry& registry, const Event& event)
	{
		// Does the target of attack already consider actor as hostile entity?
		const bool is_hostile = AlignmentSystem::is_hostile(registry, event.target.entity, event.actor.entity);
		if (!is_hostile)
		{
			// If not, they were betrayed/backstabbed, which will anger them
			AlignmentSystem::lose_opinion(registry, event.target.entity, event.actor.entity, 1);
		}
	}

	/* Is this gonna be some npc banter thing?
	 * */
	void resolve_become_hostile_event(entt::registry& registry, const Event& event)
	{
		(void) registry; (void) event;
	}

	void log_event(entt::registry& registry, Event& event)
	{
		// Shady part to make a certain test work
		if (registry.ctx().get<GameState>().test_run == false)
		{
			const bool actor_hidden = !ECS::player_can_see_entity(registry, event.actor.entity);
			const bool target_hidden = !ECS::player_can_see_entity(registry, event.target.entity);

			// Returning here means that player will not get a log entry
			if ((event.actor.entity != entt::null && actor_hidden) ||
				(event.target.entity != entt::null && target_hidden))
			{
				return;
			}
		}

		std::ostringstream stream;
		if (event.actor.entity != entt::null)
			stream << ECS::get_colored_name(registry, event.actor.entity) << " ";
		switch (event.type)
		{
			case Event::Type::Attack:
				stream << "attacks ";
				break;
			case Event::Type::TakeDamage:
				stream << "takes " << event.damage_roll << " damage";
				break;
			case Event::Type::Gather:
				stream << "gathers from ";
				break;
			case Event::Type::Equip:
				stream << "equips ";
				break;
			case Event::Type::Unequip:
				stream << "unequips ";
				break;
			case Event::Type::Drop:
				stream << "drops ";
				break;
			case Event::Type::ReceiveItem:
				stream << "receives ";
				break;
			case Event::Type::Death:
				stream << "dies";
				break;
			case Event::Type::BecomeHostile:
				stream << "becomes hostile toward ";
				break;
			case Event::Type::AttackMiss:
				stream << "doesn't hit ";
				break;
			case Event::Type::AttackHit:
				stream << "hits ";
				break;
			case Event::Type::Spawn:
			case Event::Type::Move:
				return;
			default:
				Log::warning() << "Unsupported event.type in event handler log_event: " << static_cast<size_t>(event.type);
				return;
		}
		if (event.target.entity != entt::null)
			stream << ECS::get_colored_name(registry, event.target.entity);
		if (event.type == Event::Type::Attack)
		{
			if (event.weapon != entt::null)
				stream << " (" << ECS::get_colored_name(registry, event.weapon) << ")";
			else
				stream << " (unarmed)";
		}

		event.message = stream.str();
		registry.ctx().get<EventLogger>().log_event(event);
		Log::debug() << "Event logged:" << std::endl << event;
	}

	void finalize(entt::registry& registry, Event& event)
	{
		auto& actor = event.actor;
		auto& target = event.target;

		// Derive missing position from entity's potentially existing Position component
		if (actor.entity != entt::null &&
			!actor.position.is_valid() &&
			registry.all_of<Position>(actor.entity))
			actor.position = registry.get<Position>(actor.entity);
		if (target.entity != entt::null &&
			!target.position.is_valid() &&
			registry.all_of<Position>(target.entity))
			target.position = registry.get<Position>(target.entity);
	}

	void resolve_events(entt::registry& registry)
	{
		auto& queue = registry.ctx().get<EventQueue>().queue;
		for (size_t i = 0; i < queue.size(); ++i)
		{
			auto& event = queue[i];
			finalize(registry, event);
			log_event(registry, event);
			switch (event.type)
			{
				case Event::Type::TakeDamage:
					resolve_take_damage_event(registry, event);
					break;
				case Event::Type::Move:
					resolve_move_event(registry, event);
					break;
				case Event::Type::Gather:
					resolve_gather_event(registry, event);
					break;
				case Event::Type::Drop:
				case Event::Type::Spawn:
					resolve_spawn_event(registry, event);
					break;
				/*case Event::Type::Equip:
					resolve_equip_event(registry, event);
					break;
				case Event::Type::Unequip:
					break;*/
				case Event::Type::ReceiveItem:
					resolve_receive_item_event(registry, event);
					break;
				case Event::Type::Destroy:
					resolve_destroy_event(registry, event);
					break;
				case Event::Type::Death:
					//resolve_death_event(registry, event); // Looks like event not used?
					break;
				case Event::Type::Attack:
					resolve_attack_event(registry, event);
					break;
				case Event::Type::AttackHit:
				case Event::Type::AttackMiss:
					break;
				case Event::Type::BecomeHostile:
					resolve_become_hostile_event(registry, event);
					break;
				default:
					Error::fatal("Unhandled event type: " + std::to_string(static_cast<int>(event.type)));
			}

		}
		queue.clear();

		// Destroy entities marked for destruction
		for (const auto entity : registry.view<Destroyed>())
			registry.destroy(entity);
		RenderingSystem::render(registry);
	}
};
