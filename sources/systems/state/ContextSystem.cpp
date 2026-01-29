#include <format>
#include "systems/state/AlignmentSystem.hpp"
#include "UI/MenuTxt.hpp"
#include "systems/state/StateSystem.hpp"
#include "systems/action/EventSystem.hpp"
#include "external/entt/entity/handle.hpp"
#include "systems/crafting/GatheringSystem.hpp"
#include "systems/state/ContextSystem.hpp"
#include "systems/state/InventorySystem.hpp"
#include "systems/state/EquipmentSystem.hpp"
#include "utils/ECS.hpp"
#include "UI/UI.hpp"
#include "components/Components.hpp"

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
			details.push_back(MenuTxt::HorizontalLine);
			details.insert(details.end(), resources.begin(), resources.end());
		}

		const auto& attributes = get_attribute_details(registry, entity);
		if (!attributes.empty())
		{
			details.push_back(MenuTxt::HorizontalLine);
			details.insert(details.end(), attributes.begin(), attributes.end());
		}

		const auto& alignment = get_alignment_details(registry, entity);
		if (!alignment.empty())
		{
			details.push_back(MenuTxt::HorizontalLine);
			details.insert(details.end(), alignment.begin(), alignment.end());
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
		if (distance < 1.5 && registry.all_of<Inventory>(entity))
		{
			if (player == entity)
				options.push_back("Inventory");
			else if (registry.all_of<Dead>(entity))
				options.push_back("Loot");
			else if (registry.get<Subcategory>(entity).subcategory == "furniture")
				options.push_back("Open");
		}
		if (distance < 1.5 && player != owner)
		{
			if (registry.get<Category>(entity).category == "items")
				options.push_back("Take");
		}
		if (distance < 1.5 && GatheringSystem::can_gather(registry, player, entity))
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

		EventSystem::resolve_events(registry);
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

	std::vector<std::string> get_entities_details(const entt::registry& registry, const std::vector<entt::entity>& entities, const entt::entity owner)
	{
		std::vector<std::string> entities_details;
		for (auto& item : entities)
		{
			if (EquipmentSystem::is_equipped(registry, owner, item))
				entities_details.push_back(" * " + ECS::get_colored_name(registry, item));
			else
				entities_details.push_back("   " + ECS::get_colored_name(registry, item));
		}
		return entities_details;
	}

	/* For showing either entities on the floor or in an inventory
	 * */
	bool show_entities_list(entt::registry& registry, const std::vector<entt::entity>& entities, const entt::entity owner)
	{
		if (entities.empty()) return true; // will remake later
		static size_t selection_idx = 0; // remember previous selection
		selection_idx = std::min(selection_idx, entities.size() - 1);
		const auto owner_name = owner == entt::null ? "Cell" : ECS::get_colored_name(registry, owner);
		const auto& colored_names = get_entities_details(registry, entities, owner);
		const std::string selection =
			UI::instance().dialog(
				Utils::capitalize(owner_name),
				colored_names,
				Screen::topleft(),
				selection_idx // dialog will set this value
				);
		show_entity_details(registry, entities[selection_idx], owner);
		return false;
	}

	void show_entities_list(entt::registry& registry, const entt::entity owner)
	{
		if (!registry.all_of<Inventory>(owner)) return;

		while (!registry.get<Inventory>(owner).inventory.empty())
		{
			auto& inventory = registry.get<Inventory>(owner).inventory;

			// Sort by name but also by equipped status
			std::sort(inventory.begin(), inventory.end(),
					[&](const auto a, const auto b)
					{
					const bool ae = EquipmentSystem::is_equipped(registry, owner, a);
					const bool be = EquipmentSystem::is_equipped(registry, owner, b);
					if ((ae && be) || (!ae && !be)) return registry.get<Name>(a).name < registry.get<Name>(b).name;
					return ae;
					});
			if (show_entities_list(registry, inventory, owner) == true)
				break;
		}
		if (registry.get<Inventory>(owner).inventory.empty())
			UI::instance().dialog("No items", {"Back"}, Screen::topleft());
	}
	void show_entities_list(entt::registry& registry, const Position& position)
	{
		while (!ECS::get_entities(registry, position).empty())
		{
			if (show_entities_list(registry, ECS::get_entities(registry, position), entt::null) == true)
				break;
		}
	}

	void examine_cell(entt::registry& registry, const Position& position)
	{
		const auto& entities = ECS::get_entities(registry, position);
		if (entities.empty()) return;
		if (entities.size() == 1)
			show_entity_details(registry, entities[0]);
		else
			show_entities_list(registry, position);
	}
};
