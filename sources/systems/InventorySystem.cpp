#include <string>                       // for basic_string, char_traits
#include "Cave.hpp"                     // for Cave
#include "Cell.hpp"                     // for Cell
#include "Components.hpp"               // for Position, Inventory, Name
#include "ECS.hpp"                      // for get_cell, get_colored_name
#include "UI.hpp"                       // for UI
#include "Utils.hpp"                    // for topleft, capitalize
#include "entt.hpp"                     // for vector, allocator, basic_sigh...
#include "systems/EquipmentSystem.hpp"  // for is_equipped, equip_or_unequip
#include "systems/ExamineSystem.hpp"    // for get_info_neat
#include "systems/InventorySystem.hpp"  // for add_item, has_item, inventory...

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
	}

	void take_item(entt::registry& registry, const entt::entity taker, const entt::entity owner, const entt::entity item)
	{
		remove_item(registry, owner, item);
		add_item(registry, taker, item);
	}

	void drop_item(entt::registry& registry, const entt::entity owner, const entt::entity item)
	{
		Cell* cell = ECS::get_cell(registry, owner);
		remove_item(registry, owner, item);
		registry.emplace<Position>(item, cell);
		if (registry.all_of<Glow>(item))
		{
			cell->get_cave()->reset_lights();
			cell->get_cave()->draw();
		}
	}

	size_t get_inventory_value(const entt::registry& registry, const entt::entity entity)
	{
		if (!registry.all_of<Inventory>(entity))
			return 0;

		size_t value = 0;
		const auto& inventory = registry.get<Inventory>(entity).inventory;
		for (const auto item : inventory)
			value += ECS::get_value(registry, item);
		return value;
	}
};
