#include <assert.h>
#include <format>
#include <algorithm>
#include <compare>
#include <string>
#include <vector>

#include "systems/state/AlignmentSystem.hpp"
#include "systems/state/StateSystem.hpp"
#include "systems/action/EventSystem.hpp"
#include "systems/crafting/GatheringSystem.hpp"
#include "systems/state/ContextSystem.hpp"
#include "systems/state/InventorySystem.hpp"
#include "systems/state/EquipmentSystem.hpp"
#include "utils/ECS.hpp"
#include "components/Components.hpp"
#include "UI/Dialog.hpp"
#include "UI/Menu.hpp"
#include "domain/Cell.hpp"
#include "domain/Color.hpp"
#include "domain/LiquidMixture.hpp"
#include "external/entt/entity/fwd.hpp"
#include "external/entt/entt.hpp"
#include "utils/Screen.hpp"

struct Position;

namespace ContextSystem
{
	std::vector<std::string> get_attribute_details(const entt::registry& registry, const entt::entity entity)
	{
		std::vector<std::string> details;
		if (registry.all_of<Vitality>(entity))
			details.push_back("Vitality : " + std::to_string(StateSystem::get_attribute<Vitality>(registry, entity)));
		if (registry.all_of<Endurance>(entity))
			details.push_back("Endurance : " + std::to_string(StateSystem::get_attribute<Endurance>(registry, entity)));
		if (registry.all_of<Willpower>(entity))
			details.push_back("Willpower : " + std::to_string(StateSystem::get_attribute<Willpower>(registry, entity)));
		if (registry.all_of<Perception>(entity))
			details.push_back("Perception : " + std::to_string(StateSystem::get_attribute<Perception>(registry, entity)));
		if (registry.all_of<Charisma>(entity))
			details.push_back("Charisma : " + std::to_string(StateSystem::get_attribute<Charisma>(registry, entity)));
		return details;
	}
	std::vector<std::string> get_resource_details(const entt::registry& registry, const entt::entity entity)
	{
		std::vector<std::string> details;
		if (registry.all_of<Health>(entity))
		{
			std::string health = "Health : " + std::to_string(registry.get<Health>(entity).current);
			if (registry.all_of<Vitality>(entity))
				health += " / " + std::to_string(StateSystem::get_max_health(registry, entity));
			details.push_back(health);
		}
		if (registry.all_of<Stamina>(entity))
		{
			std::string stamina = "Stamina : " + std::to_string(registry.get<Stamina>(entity).current);
			if (registry.all_of<Endurance>(entity))
				stamina += " / " + std::to_string(StateSystem::get_max_stamina(registry, entity));
			details.push_back(stamina);
		}
		if (registry.all_of<Mana>(entity))
		{
			std::string mana = "Mana : " + std::to_string(registry.get<Mana>(entity).current);
			if (registry.all_of<Willpower>(entity))
				mana += " / " + std::to_string(StateSystem::get_max_mana(registry, entity));
			details.push_back(mana);
		}
		return details;
	}

	std::vector<std::string> get_alignment_details(const entt::registry& registry, const entt::entity entity)
	{
		if (!registry.all_of<Alignment>(entity))
			return {};
		std::vector<std::string> details;

		if (entity != ECS::get_player(registry))
		{
			details.push_back(registry.get<Alignment>(entity).to_string());
			if (AlignmentSystem::is_hostile(registry, entity, ECS::get_player(registry)))
				details.push_back(Color::red().markup() + "Hostile{reset}");
			else if (AlignmentSystem::is_friendly(registry, entity, ECS::get_player(registry)))
				details.push_back(Color::green().markup() + "Friendly{reset}");
			else
				details.push_back("Neutral");
			return details;
		}

		const auto& alignment = registry.get<Alignment>(entity);
		if (alignment.chaos_law <= -0.5)
			details.push_back("Chaotic : " + std::format("{:.2f}", alignment.chaos_law));
		else if (alignment.chaos_law >= 0.5)
			details.push_back("Lawful : " + std::format("{:.2f}", alignment.chaos_law));
		else
			details.push_back("Neutral : " + std::format("{:.2f}", alignment.chaos_law));
		if (alignment.evil_good <= -0.5)
			details.push_back("Evil : " + std::format("{:.2f}", alignment.evil_good));
		else if (alignment.evil_good >= 0.5)
			details.push_back("Good : " + std::format("{:.2f}", alignment.evil_good));
		else
			details.push_back("Neutral : " + std::format("{:.2f}", alignment.evil_good));
		if (entity != ECS::get_player(registry))
			details.push_back("Tolerance : " + std::format("{:.2f}", alignment.tolerance));
		return details;
	}

	std::vector<std::string> get_details(const entt::registry& registry, const entt::entity entity)
	{
		std::vector<std::string> details = { ECS::get_colored_name(registry, entity) };
		details.push_back(registry.get<Category>(entity).category + " / " + registry.get<Subcategory>(entity).subcategory);

		const auto& resources = get_resource_details(registry, entity);
		if (!resources.empty())
		{
			details.push_back("--");
			details.insert(details.end(), resources.begin(), resources.end());
		}

		const auto& attributes = get_attribute_details(registry, entity);
		if (!attributes.empty())
		{
			details.push_back("--");
			details.insert(details.end(), attributes.begin(), attributes.end());
		}

		const auto& alignment = get_alignment_details(registry, entity);
		if (!alignment.empty())
		{
			details.push_back("--");
			details.insert(details.end(), alignment.begin(), alignment.end());
		}

		return details;
	}

	std::vector<std::string> get_options(const entt::registry& registry, const entt::entity entity, const entt::entity owner)
	{
		assert(registry.all_of<Position>(entity) || InventorySystem::has_item(registry, owner, entity));
		const double distance = !registry.all_of<Position>(entity) ?
			ECS::distance(registry, ECS::get_player(registry), owner) :
			ECS::distance(registry, ECS::get_player(registry), entity);
		const bool is_player = entity == ECS::get_player(registry);
		const bool is_player_owned = owner == ECS::get_player(registry);
		std::vector<std::string> options;
		if (distance < 1.5)
		{
			if (!is_player_owned && registry.get<Category>(entity).category == "items")
				options.push_back("Take");
			if (!is_player && registry.all_of<Inventory, Position>(entity))
				options.push_back("Loot");
			if (GatheringSystem::can_gather(registry, ECS::get_player(registry), entity))
				options.push_back("Gather");
		}
		if (is_player_owned)
		{
			options.push_back("Drop");
			if (registry.all_of<Equipment>(entity))
			{
				if (EquipmentSystem::is_equipped(registry, ECS::get_player(registry), entity))
					options.push_back("Unequip");
				else
					options.push_back("Equip");
			}
		}
		if (is_player)
			options.push_back("Inventory");
		return options;
	}

	/* Take - take item to players inventory from ground or other inventory
	 * Drop - drop item from own inventory to ground
	 * Loot - open inventory of dead creature or container
	 * Inventory - open own inventory (optional, i hotkey)
	 * Equip - equip item with logic from EquipmentSystem if it is in players inventory
	 * Unequip - same but different
	 * Take liquid - take liquid to bottle
	 * Gather - gather resources (cutting, felling, mining...)
	 * Rest - start resting (optional, r hotkey)
	 * Hide - go in stealth mode (optional, h hotkey)
	 *
	 * */
	void handle_selection(entt::registry& registry, const entt::entity entity, const std::string& label, const entt::entity owner)
	{
		if (label == "Take")
			InventorySystem::take_item(registry, ECS::get_player(registry), owner, entity);
		else if (label == "Drop")
			InventorySystem::drop_item(registry, ECS::get_player(registry), entity);
		else if (label == "Loot" || label == "Inventory")
			open_inventory(registry, entity);
		else if (label == "Equip" || label == "Unequip")
			EquipmentSystem::equip_or_unequip(registry, ECS::get_player(registry), entity);
		//else if (label == "Take liquid")
		//	This needs some kind of UI thing, use same as the Enter examine UI
		else if (label == "Gather")
			GatheringSystem::gather(registry, ECS::get_player(registry), entity);
		EventSystem::resolve_events(registry); // Do this so that log messages will show while menu is open
	}

	void show_entity_details(entt::registry& registry, const entt::entity entity, const entt::entity owner)
	{
		auto text = get_details(registry, entity);
		auto buttons = get_options(registry, entity, owner);
		buttons.push_back("Back");
		const auto selection = Dialog::get_selection(text, buttons, Screen::topleft());
		handle_selection(registry, entity, selection.label, owner);
	}

	std::vector<std::string> get_inventory_format(const entt::registry& registry, const std::vector<entt::entity>& inventory, const entt::entity entity)
	{
		std::vector<std::string> formatted;
		for (const auto item : inventory)
		{
			std::string str = "";
			if (EquipmentSystem::is_equipped(registry, entity, item))
				str += " * ";
			else
				str += "   ";
			str += ECS::get_colored_name(registry, item);
			formatted.push_back(str);
		}
		return formatted;
	}

	void open_inventory(entt::registry& registry, const entt::entity entity)
	{
		if (!registry.all_of<Inventory>(entity))
			return;
		auto& inventory_component = registry.get<Inventory>(entity);
		auto& inventory = inventory_component.items;

		Menu::Element selection;
		while (true)
		{
			std::sort(inventory.begin(), inventory.end(), [&](const auto a, const auto b) {
					const bool ae = EquipmentSystem::is_equipped(registry, entity, a);
					const bool be = EquipmentSystem::is_equipped(registry, entity, b);
					if ((ae && be) || (!ae && !be)) return registry.get<Name>(a).name < registry.get<Name>(b).name;
					return ae;
					});
			if (inventory.empty())
			{
				Dialog::get_selection("Inventory empty", {"OK"}, Screen::topleft());
				break;
			}
			std::vector<std::string> text;
			text.push_back(ECS::get_colored_name(registry, entity) + "s Inventory");
			std::vector<std::string> buttons = get_inventory_format(registry, inventory, entity);
			buttons.push_back("Back");
			selection = Dialog::get_selection(text, buttons, Screen::topleft(), selection.index);
			if (selection.label == "Back" || selection.label.empty())
				break;
			show_entity_details(registry, inventory[selection.index], entity);
		}
	}

	void examine_cell(entt::registry& registry, const Position& position)
	{
		const auto& cell = ECS::get_cell(registry, position);
		const auto& mixture = cell.get_liquid_mixture();

		Menu::Element selection;
		while (true)
		{
			std::vector<std::string> text;
			text.push_back(cell.to_string());
			text.push_back(mixture.to_string());
			auto entities = ECS::get_entities(registry, position);
			std::sort(entities.begin(), entities.end(), [&](const auto a, const auto b) {
					return ECS::get_name(registry, a) < ECS::get_name(registry, b);
					});
			std::vector<std::string> buttons = ECS::get_colored_names(registry, entities);
			buttons.push_back("Back");
			selection = Dialog::get_selection(text, buttons, Screen::topleft(), selection.index);
			if (selection.label == "Back" || selection.label.empty())
				break;
			show_entity_details(registry, entities[selection.index]);
		}
	}
};
