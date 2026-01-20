#include <string>                       // for basic_string, char_traits
#include "components/Components.hpp"               // for Position, Inventory, Name
#include "utils/ECS.hpp"                      // for get_cell, get_colored_name
#include "external/entt/entt.hpp"                     // for vector, allocator, basic_sigh...
#include "utils/Log.hpp"
#include "systems/state/EquipmentSystem.hpp"  // for is_equipped, equip_or_unequip
#include "systems/state/InventorySystem.hpp"  // for add_item, has_item, inventory...

namespace InventorySystem
{
	bool has_item(const entt::registry& registry, const entt::entity holder, const entt::entity item)
	{
		const auto& inventory = registry.get<Inventory>(holder).inventory;
		auto it = std::find(inventory.begin(), inventory.end(), item);
		return it != inventory.end();
	}

	void remove_item(entt::registry& registry, const entt::entity owner, const entt::entity item)
	{
		if (owner == entt::null)
		{
			registry.erase<Position>(item);
			return;
		}

		if (EquipmentSystem::is_equipped(registry, owner, item))
			EquipmentSystem::unequip(registry, owner, item);
		auto& inventory = registry.get<Inventory>(owner).inventory;
		auto it = std::find(inventory.begin(), inventory.end(), item);
		inventory.erase(it);
	}

	void add_item(entt::registry& registry, const entt::entity entity, const entt::entity item)
	{
		auto& inventory = registry.get<Inventory>(entity).inventory;
		inventory.push_back(item);
		ECS::queue_event(registry, Event(
					{.entity = entity},
					{.type = Effect::Type::ReceiveItem},
					{.entity = item}
					));
	}

	void take_item(entt::registry& registry, const entt::entity taker, const entt::entity owner, const entt::entity item)
	{
		remove_item(registry, owner, item);
		add_item(registry, taker, item);
	}

	void drop_item(entt::registry& registry, const entt::entity owner, const entt::entity item)
	{
		const Position& pos = registry.get<Position>(owner);
		remove_item(registry, owner, item);
		registry.emplace_or_replace<Position>(item, pos);

		// This will log message, and apply f.e. Glow effects.
		Event drop_event;
		drop_event.actor.entity = owner;
		drop_event.effect.type = Effect::Type::Drop;
		drop_event.target.entity = item;
		ECS::queue_event(registry, drop_event);
	}

	bool has_inventory(const entt::registry& registry, const entt::entity entity)
	{
		return registry.all_of<Inventory>(entity);
	}

};
