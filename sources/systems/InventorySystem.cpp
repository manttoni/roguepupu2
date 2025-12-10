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
	bool inventory_key_pressed(const int key)
	{
		return key == 'i';
	}

	std::vector<std::string> get_colored_item_names(const entt::registry& registry, const std::vector<entt::entity>& items)
	{
		auto player = *registry.view<Player>().begin();
		std::vector<std::string> names;
		for (const auto& item : items)
		{
			std::string name = EquipmentSystem::is_equipped(registry, player, item) ? " * " : "   ";
			name += ECS::get_colored_name(registry, item);
			names.push_back(name);
		}
		return names;
	}

	bool has_item(const entt::registry& registry, const entt::entity holder, const entt::entity item)
	{
		const auto& inventory = registry.get<Inventory>(holder).inventory;
		auto it = std::find(inventory.begin(), inventory.end(), item);
		return it != inventory.end();
	}

	void remove_item(entt::registry& registry, const entt::entity entity, const entt::entity item)
	{
		if (EquipmentSystem::is_equipped(registry, entity, item))
			EquipmentSystem::unequip(registry, entity, item);
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
		remove_item(registry, owner, item);
		add_item(registry, looter, item);
	}

	void drop_item(entt::registry& registry, const entt::entity owner, const entt::entity item)
	{
		remove_item(registry, owner, item);
		Cell* cell = ECS::get_cell(registry, owner);
		registry.emplace<Position>(item, cell);
		if (registry.all_of<Glow>(item))
		{
			cell->get_cave()->reset_lights();
			cell->get_cave()->draw();
		}
	}

	std::vector<std::string> get_options(const entt::registry& registry, const entt::entity owner, const entt::entity item)
	{
		auto player = *registry.view<Player>().begin();
		std::vector<std::string> options;
		if (owner == player)
		{
			if (registry.any_of<Weapon, Armor>(item)) // can be equipped
			{
				if (EquipmentSystem::is_equipped(registry, player, item))
					options.push_back("Unequip");
				else
					options.push_back("Equip");
			}
		}
		else options.push_back("Loot");
		options.push_back("Drop");
		options.push_back("Cancel");
		return options;
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
			// Create list of items to show in inventory window
			auto& items = registry.get<Inventory>(owner).inventory;
			const auto& names = get_colored_item_names(registry, items);

			// Dialog returns selection
			selection = UI::instance().dialog("*** " + Utils::capitalize(owner_name) + " ***", names, Screen::topleft(), idx + 1);
			if (selection.empty()) // Esc was pressed
				break;

			// Get index of selected item
			auto it = std::find(names.begin(), names.end(), selection);
			idx = it - names.begin();
			auto& item = items[idx];

			const auto& options = get_options(registry, owner, item);
			const auto& info_neat = ExamineSystem::get_info_neat(registry, item);
			selection = UI::instance().dialog(info_neat, options, Screen::topleft());
			if (selection == "Equip" || selection == "Unequip")
				EquipmentSystem::equip_or_unequip(registry, player, item);
			else if (selection == "Loot")
				loot_item(registry, player, owner, item);
			else if (selection == "Drop")
				drop_item(registry, owner, item);
		}
	}
};
