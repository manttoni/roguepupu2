#include <assert.h>
#include <curses.h>
#include <stddef.h>
#include <optional>
#include <string>
#include <vector>

#include "components/Components.hpp"
#include "domain/Actor.hpp"
#include "domain/Damage.hpp"
#include "domain/Effect.hpp"
#include "domain/Event.hpp"
#include "domain/EventQueue.hpp"
#include "domain/Position.hpp"
#include "domain/Target.hpp"
#include "external/entt/entity/fwd.hpp"
#include "external/entt/entt.hpp"
#include "infrastructure/GameLogger.hpp"
#include "systems/action/EventSystem.hpp"
#include "systems/items/LootSystem.hpp"
#include "systems/position/TransitionSystem.hpp"
#include "systems/rendering/LightingSystem.hpp"
#include "systems/rendering/RenderingSystem.hpp"
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
		switch (comp.effect)
		{
			case Gatherable::Effect::None:
				break;
			case Gatherable::Effect::Dim:
				registry.emplace_or_replace<NcursesAttr>(event.target.entity, A_DIM);
				if (registry.all_of<Glow>(event.target.entity))
				{
					registry.erase<Glow>(event.target.entity);
					LightingSystem::reset_lights(registry, event.target.position.cave_idx);
				}
				break;
			case Gatherable::Effect::Destroy:
				ECS::destroy_entity(registry, event.target.entity);
				break;
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

	void resolve_equip_event(entt::registry& registry, const Event& event)
	{
		const auto equipper = event.actor.entity;
		const auto item = event.target.entity;
		if (registry.all_of<Buff<Strength>>(item)) StateSystem::add_buff(registry, equipper, registry.get<Buff<Strength>>(item));
		else if (registry.all_of<Buff<Dexterity>>(item)) StateSystem::add_buff(registry, equipper, registry.get<Buff<Dexterity>>(item));
		else if (registry.all_of<Buff<Agility>>(item)) StateSystem::add_buff(registry, equipper, registry.get<Buff<Agility>>(item));
		else if (registry.all_of<Buff<Perception>>(item)) StateSystem::add_buff(registry, equipper, registry.get<Buff<Perception>>(item));
		else if (registry.all_of<Buff<Vitality>>(item)) StateSystem::add_buff(registry, equipper, registry.get<Buff<Vitality>>(item));
		else if (registry.all_of<Buff<Endurance>>(item)) StateSystem::add_buff(registry, equipper, registry.get<Buff<Endurance>>(item));
		else if (registry.all_of<Buff<Willpower>>(item)) StateSystem::add_buff(registry, equipper, registry.get<Buff<Willpower>>(item));
		else if (registry.all_of<Buff<Charisma>>(item)) StateSystem::add_buff(registry, equipper, registry.get<Buff<Charisma>>(item));
	}

	void resolve_unequip_event(entt::registry& registry, const Event& event)
	{
		const auto unequipper = event.actor.entity;
		const auto item = event.target.entity;
		if (registry.all_of<Buff<Strength>>(item)) StateSystem::remove_buff(registry, unequipper, registry.get<Buff<Strength>>(item));
		else if (registry.all_of<Buff<Dexterity>>(item)) StateSystem::remove_buff(registry, unequipper, registry.get<Buff<Dexterity>>(item));
		else if (registry.all_of<Buff<Agility>>(item)) StateSystem::remove_buff(registry, unequipper, registry.get<Buff<Agility>>(item));
		else if (registry.all_of<Buff<Perception>>(item)) StateSystem::remove_buff(registry, unequipper, registry.get<Buff<Perception>>(item));
		else if (registry.all_of<Buff<Vitality>>(item)) StateSystem::remove_buff(registry, unequipper, registry.get<Buff<Vitality>>(item));
		else if (registry.all_of<Buff<Endurance>>(item)) StateSystem::remove_buff(registry, unequipper, registry.get<Buff<Endurance>>(item));
		else if (registry.all_of<Buff<Willpower>>(item)) StateSystem::remove_buff(registry, unequipper, registry.get<Buff<Willpower>>(item));
		else if (registry.all_of<Buff<Charisma>>(item)) StateSystem::remove_buff(registry, unequipper, registry.get<Buff<Charisma>>(item));
	}


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
			if (!registry.all_of<EquipmentSlots>(entity) || !registry.all_of<Equipment>(item))
				return;
			Log::log("Equipping item: " + registry.get<Name>(item).name);
			EquipmentSystem::equip_in_free_slots(registry, entity, item);
		}
	}

	void log_event(entt::registry& registry, const Event& event)
	{
		Log::log("event.type: " + std::to_string(static_cast<size_t>(event.type)));
		std::string message;
		if (event.actor.entity != entt::null)
			message += ECS::get_colored_name(registry, event.actor.entity) + " ";
		switch (event.type)
		{
			case Event::Type::Attack:
				message += "attacks ";
				break;
			case Event::Type::TakeDamage:
				message += "takes " + event.damage_roll.to_string() + " damage";
				break;
			case Event::Type::Gather:
				message += "gathers from ";
				break;
			case Event::Type::Equip:
				message += "equips ";
				break;
			case Event::Type::Unequip:
				message += "unequips ";
				break;
			case Event::Type::Drop:
				message += "drops ";
				break;
			case Event::Type::ReceiveItem:
				message += "receives ";
				break;
			case Event::Type::Death:
				message += "dies";
				break;
			default:
				return;
		}
		if (event.target.entity != entt::null)
			message += ECS::get_colored_name(registry, event.target.entity);
		if (event.type == Event::Type::Attack)
		{
			if (event.weapon != entt::null)
				message += " (" + ECS::get_colored_name(registry, event.weapon) + ")";
			else
				message += " (unarmed)";
		}
		if (event.type == Event::Type::Attack)
		{
			if (event.hit_quality < 0)
				message += ", but misses";
		}
		registry.ctx().get<GameLogger>().log(message);
		Log::log("Game Log: " + message);
	}

	void resolve_events(entt::registry& registry)
	{
		auto& queue = registry.ctx().get<EventQueue>().queue;
		Log::log("Resolving events (" + std::to_string(queue.size()) + ")");
		for (size_t i = 0; i < queue.size(); ++i)
		{
			const auto& event = queue[i];
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
				case Event::Type::Equip:
					resolve_equip_event(registry, event);
					break;
				case Event::Type::Unequip:
					break;
				case Event::Type::ReceiveItem:
					Log::log("Event receive");
					resolve_receive_item_event(registry, event);
					break;
				case Event::Type::Destroy:
					resolve_destroy_event(registry, event);
					break;
				case Event::Type::Death:
					//resolve_death_event(registry, event);
					break;
				case Event::Type::Attack:
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
