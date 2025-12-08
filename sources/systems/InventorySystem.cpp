#include <vector>
#include <string>
#include "Utils.hpp"
#include "Components.hpp"
#include "entt.hpp"
#include "systems/InventorySystem.hpp"
#include "systems/EquipmentSystem.hpp"
#include "UI.hpp"
#include "ECS.hpp"

namespace InventorySystem
{
	bool inventory_key_pressed(const int key)
	{
		return key == 'i';
	}

	std::vector<std::string> create_descriptions(const entt::registry& registry, const std::vector<entt::entity>& items)
	{
		auto player = *registry.view<Player>().begin();
		std::vector<std::string> descriptions;
		for (const auto& item : items)
		{
			std::string description = EquipmentSystem::is_equipped(registry, player, item) ? " * " : "   ";
			description += ECS::get_description(registry, item);
			descriptions.push_back(description);
		}
		return descriptions;
	}

	bool has_item(const entt::registry& registry, const entt::entity holder, const entt::entity item)
	{
		if (!registry.all_of<Inventory>(holder))
			Log::error("Checking non-existent inventory of: " + ECS::get_name(registry, holder));
		const auto& inventory = registry.get<Inventory>(holder).inventory;
		auto it = std::find(inventory.begin(), inventory.end(), item);
		return it != inventory.end();
	}

	void remove_item(entt::registry& registry, const entt::entity entity, const entt::entity item)
	{
		auto& inventory = registry.get<Inventory>(entity).inventory;
		auto it = std::find(inventory.begin(), inventory.end(), item);
		inventory.erase(it);
	}

	void add_item(entt::registry& registry, const entt::entity entity, const entt::entity item)
	{
		auto& inventory = registry.get<Inventory>(entity).inventory;
		inventory.push_back(item);
	}

	void loot_item(entt::registry& registry, const entt::entity looter, const entt::entity owner, const entt::entity item)
	{
		if (!has_item(registry, owner, item))
			Log::error("Looting non-existent item from: " + ECS::get_name(registry, owner));

		remove_item(registry, owner, item);
		add_item(registry, looter, item);
	}

	void open_inventory(entt::registry& registry, const entt::entity owner)
	{
		if (!registry.all_of<Inventory>(owner))
			return;

		const auto& owner_name = registry.get<Name>(owner).name;
		auto player = *registry.view<Player>().begin();
		std::string selection = "none";
		size_t idx = 0;
		while (selection != "") // Will be "" when esc pressed in dialog
		{
			auto& items = registry.get<Inventory>(owner).inventory;
			const auto& descriptions = create_descriptions(registry, items);
			selection = UI::instance().dialog("*** " + Utils::capitalize(owner_name) + " ***", descriptions, Screen::top_left(), idx + 1);
			if (selection.empty())
				break;
			auto it = std::find(descriptions.begin(), descriptions.end(), selection);
			idx = it - descriptions.begin();
			auto& item = items[idx];

			if (owner == player)
			{
				if (registry.any_of<Weapon, Armor>(item))
					EquipmentSystem::equip_or_unequip(registry, player, item);
			}
			else
			{
				loot_item(registry, player, owner, item);
			}
		}
	}
};
