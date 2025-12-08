#include <string>                       // for basic_string, operator+, char...
#include "Components.hpp"               // for Inventory, Name, Player (ptr ...
#include "ECS.hpp"                      // for get_name, get_description
#include "UI.hpp"                       // for UI
#include "Utils.hpp"                    // for error, capitalize, top_left
#include "entt.hpp"                     // for vector, allocator, basic_sigh...
#include "systems/EquipmentSystem.hpp"  // for equip_or_unequip, is_equipped
#include "systems/InventorySystem.hpp"  // for add_item, has_item, inventory...

namespace InventorySystem
{
	bool inventory_key_pressed(const int key)
	{
		return key == 'i';
	}

	std::vector<std::string> get_item_names(const entt::registry& registry, const std::vector<entt::entity>& items)
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

	std::string get_proficiency(const entt::registry& registry, const entt::entity item)
	{
		std::string proficiency = "";
		if (registry.all_of<Weapon>(item))
			proficiency = registry.get<Weapon>(item).proficiency + " weapons";
		if (registry.all_of<Armor>(item))
			proficiency = registry.get<Armor>(item).proficiency + " armor";
		return Utils::capitalize(proficiency);
	}

	std::string get_damage(const entt::registry& registry, const entt::entity item)
	{
		std::string damage = "", versatile_damage = "";
		if (registry.all_of<Damage>(item))
		{
			const auto& dmg_cmp = registry.get<Damage>(item);
			damage += dmg_cmp.dice.get_string() + " ";
			if (registry.all_of<Weapon>(item))
				versatile_damage = registry.get<Weapon>(item).versatile_dice.get_string();
			if (!versatile_damage.empty())
				damage += "( " + versatile_damage + " ) ";
			damage += dmg_cmp.type;
		}
		return Utils::capitalize(damage);
	}

	std::string get_armor_class(const entt::registry& registry, const entt::entity item)
	{
		if (registry.all_of<Armor>(item))
			return std::to_string(registry.get<Armor>(item).armor_class);
		return "";
	}

	std::vector<std::string> get_item_stats(const entt::registry& registry, const entt::entity item)
	{
		std::vector<std::string> stats = {"*** " + ECS::get_colored_name(registry, item) + " ***"};

		std::string proficiency = get_proficiency(registry, item);
		if (!proficiency.empty())
			stats.push_back("Proficiency: " + proficiency);

		std::string damage = get_damage(registry, item);
		if (!damage.empty())
			stats.push_back("Damage: " + damage);

		std::string armor_class = get_armor_class(registry, item);
		if (!armor_class.empty())
			stats.push_back("AC: " + armor_class);

		return stats;
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
			const auto& names = get_item_names(registry, items);

			// Dialog returns selection
			selection = UI::instance().dialog("*** " + Utils::capitalize(owner_name) + " ***", names, Screen::top_left(), idx + 1);
			if (selection.empty()) // Esc was pressed
				break;

			// Get index of selected item
			auto it = std::find(names.begin(), names.end(), selection);
			idx = it - names.begin();
			auto& item = items[idx];

			// Show stats and options
			const auto& stats = get_item_stats(registry, item);
			const auto& options = get_options(registry, owner, item);
			selection = UI::instance().dialog(stats, options, Screen::top_left());
			if (selection == "Equip" || selection == "Unequip")
				EquipmentSystem::equip_or_unequip(registry, player, item);
			else if (selection == "Loot")
				loot_item(registry, player, owner, item);
		}
	}
};
