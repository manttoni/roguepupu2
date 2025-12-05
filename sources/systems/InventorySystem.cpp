#include <vector>
#include <string>
#include "Utils.hpp"
#include "Components.hpp"
#include "entt.hpp"
#include "systems/InventorySystem.hpp"
#include "systems/EquipmentSystem.hpp"
#include "UI.hpp"

namespace InventorySystem
{
	bool inventory_key_pressed(const int key)
	{
		return key == 'i';
	}

	std::vector<std::string> create_options(const entt::registry& registry, const std::vector<entt::entity>& items)
	{
		auto player = *registry.view<Player>().begin();
		std::vector<std::string> item_names;
		for (const auto& item : items)
		{
			auto name = registry.get<Name>(item).name;
			if (EquipmentSystem::is_equipped(registry, player, item))
				name.insert(0, " * ");
			else
				name.insert(0, "   ");
			item_names.push_back(name);
		}
		return item_names;
	}

	entt::entity get_item_by_id(const entt::registry& registry, const std::string& id, const std::vector<entt::entity>& items)
	{
		for (const auto& item : items)
		{
			if (registry.get<Name>(item).name == id)
				return item;
		}
		return entt::null;
	}

	void open_inventory(entt::registry& registry, const entt::entity entity)
	{
		if (!registry.all_of<Inventory>(entity))
			return;

		auto player = *registry.view<Player>().begin();
		std::string selection = "none";
		while (selection != "") // Will be "" when esc pressed in dialog
		{
			auto& items = registry.get<Inventory>(entity).inventory;
			const auto& item_names = create_options(registry, items);
			selection = UI::instance().dialog("** Inventory **", item_names, Screen::top_left());
			if (selection.empty())
				break;
			entt::entity item = get_item_by_id(registry, selection.substr(3), items);
			if (item == entt::null)
				break;

			if (registry.all_of<Equippable>(item))
			{
				if (!EquipmentSystem::is_equipped(registry, player, item))
					EquipmentSystem::equip(registry, player, item);
				else
					EquipmentSystem::unequip(registry, player, item);
			}
		}
	}
};
