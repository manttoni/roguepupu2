#include <format>
#include "systems/GatheringSystem.hpp"
#include "systems/ContextSystem.hpp"
#include "systems/InventorySystem.hpp"
#include "systems/EquipmentSystem.hpp"
#include "ECS.hpp"
#include "UI.hpp"
#include "Utils.hpp"
#include "Components.hpp"
#include "Cell.hpp"

#define MELEE_RANGE 1.5

namespace ContextSystem
{
	std::vector<std::string> get_details(const entt::registry& registry, const entt::entity entity)
	{
		const auto& info = ECS::get_info(registry, entity);
		size_t leftcol = 0;
		size_t rightcol = 0;
		for (const auto& [left, right] : info)
		{
			leftcol = std::max(leftcol, left.size());
			rightcol = std::max(rightcol, right.size());
		}
		std::vector<std::string> details = { ECS::get_colored_name(registry, entity) };
		for (const auto& [left, right] : info)
		{
			const std::string line = std::format("{: <{}} : {: >{}}", left, leftcol, right, rightcol);
			details.push_back(Utils::capitalize(line));
		}
		return details;
	}
	std::vector<std::string> get_options(const entt::registry& registry, const entt::entity entity, const entt::entity owner)
	{
		std::vector<std::string> options;
		const auto player = ECS::get_player(registry);
		if (player == owner && owner != entt::null)
		{
			if (registry.all_of<Equipment>(entity))
			{
				if (EquipmentSystem::is_equipped(registry, player, entity))
					options.push_back("Unequip");
				else
					options.push_back("Equip");
			}
			options.push_back("Drop");
		}

		double distance;
		if (registry.all_of<Position>(entity))
			distance = ECS::distance(registry, player, entity);
		else
			distance = ECS::distance(registry, player, owner);
		if (distance < MELEE_RANGE && registry.all_of<Inventory>(entity))
		{
			if (player == entity)
				options.push_back("Inventory");
			else if (registry.all_of<Dead>(entity))
				options.push_back("Loot");
			else if (registry.get<Subcategory>(entity).subcategory == "furniture")
				options.push_back("Open");
		}
		if (distance < MELEE_RANGE && player != owner)
		{
			if (registry.get<Category>(entity).category == "items")
				options.push_back("Take");
		}
		if (distance < MELEE_RANGE && GatheringSystem::can_gather(registry, player, entity))
			options.push_back("Gather");
		options.push_back("Back");
		return options;
	}
	void handle_selection(entt::registry& registry, const entt::entity entity, const entt::entity owner, const std::string& selection)
	{
		const auto player = ECS::get_player(registry);
		if (selection == "Unequip" || selection == "Equip")
			EquipmentSystem::equip_or_unequip(registry, player, entity);
		if (selection == "Inventory" || selection == "Open" || selection == "Loot")
			show_entities_list(registry, entity);
		if (selection == "Drop")
			InventorySystem::drop_item(registry, player, entity);
		if (selection == "Take")
			InventorySystem::take_item(registry, player, owner, entity);
		if (selection == "Gather")
			GatheringSystem::gather(registry, player, entity);
	}
	void show_entity_details(entt::registry& registry, const entt::entity entity, const entt::entity owner)
	{
		while (true)
		{
			const auto& details = get_details(registry, entity);
			const auto& options = get_options(registry, entity, owner);
			const std::string selection = UI::instance().dialog(details, options, Screen::topleft());
			handle_selection(registry, entity, owner, selection);
			if (selection != "Inventory") break;
		}
	}
	bool show_entities_list(entt::registry& registry, const std::vector<entt::entity>& entities, const entt::entity owner)
	{
		static size_t selection_idx = 0; // remember previous selection
		selection_idx = std::min(selection_idx, entities.size() - 1);
		const auto owner_name = owner == entt::null ? "Cell" : ECS::get_colored_name(registry, owner);
		const auto& colored_names = ECS::get_colored_names(registry, entities);
		const std::string selection =
			UI::instance().dialog(
				Utils::capitalize(owner_name),
				colored_names,
				Screen::topleft(),
				selection_idx
				);
		auto it = std::find(colored_names.begin(), colored_names.end(), selection);
		if (it == colored_names.end()) return true;
		selection_idx = std::distance(colored_names.begin(), it);
		show_entity_details(registry, entities[selection_idx], owner);
		return false;
	}
	void show_entities_list(entt::registry& registry, const entt::entity owner)
	{
		if (!registry.all_of<Inventory>(owner)) return;
		while (!ECS::get_inventory(registry, owner).empty())
			if (show_entities_list(registry, ECS::get_inventory(registry, owner), owner) == true)
				break;
		if (ECS::get_inventory(registry, owner).empty())
			UI::instance().dialog("No items", {"Back"}, Screen::topleft());
	}
	void show_entities_list(entt::registry& registry, Cell* cell)
	{
		if (cell == nullptr) return;
		while (!cell->get_entities().empty())
			if (show_entities_list(registry, cell->get_entities(), entt::null) == true)
				break;
	}

	void examine_cell(entt::registry& registry, Cell* cell)
	{
		if (cell == nullptr) return;
		const auto entities = cell->get_entities();
		if (entities.empty()) return;
		if (entities.size() == 1)
			show_entity_details(registry, entities[0]);
		else
			show_entities_list(registry, cell);
	}
};
