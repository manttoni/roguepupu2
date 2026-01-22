#include <vector>
#include <string>
#include "database/EntityFactory.hpp"
#include "infrastructure/DevTools.hpp"
#include "UI/UI.hpp"
#include "external/entt/entt.hpp"
#include "utils/ECS.hpp"
#include "domain/Cave.hpp"
#include "domain/Cell.hpp"
#include "components/Components.hpp"
#include "domain/Color.hpp"

namespace DevTools
{
	std::string get_amount(const entt::registry& registry, const std::string& category)
	{
		auto all = registry.view<Category>();
		std::vector<entt::entity> matches;
		for (const auto e : all)
		{
			if (registry.get<Category>(e).category == category ||
					registry.get<Subcategory>(e).subcategory == category)
				matches.push_back(e);
		}
		return std::to_string(matches.size()) + " " + category;
	}

	void show_entities(const entt::registry& registry)
	{
		std::map<std::string, size_t> groups;
		auto all = registry.view<Category>();
		for (const auto e : all)
		{
			const auto& subcategory = registry.get<Subcategory>(e).subcategory;
			groups[subcategory]++;
		}
		std::vector<std::string> data;
		for (const auto& [category, amount] : groups)
			data.push_back(std::to_string(amount) + " " + category);
		UI::instance().dialog(data, {"Back"});
	}

	void change_liquid(entt::registry& registry)
	{
		std::vector<std::string> choices;
		for (size_t i = static_cast<size_t>(Liquid::Type::None) + 1;
				i < static_cast<size_t>(Liquid::Type::Count); ++i)
		{
			choices.push_back(Liquid::to_string(static_cast<Liquid::Type>(i)));
		}
		registry.ctx().get<Dev>().liquid_type = Liquid::from_string(
				UI::instance().dialog("Choose liquid", choices));
	}

	void spawn_entity(entt::registry& registry)
	{
		const std::vector<std::string> categories = EntityFactory::instance().get_category_names();
		const std::string category = UI::instance().dialog("Choose category", categories);
		if (category.empty()) return;
		const std::vector<std::string> subcategories = EntityFactory::instance().get_subcategory_names(category);
		const std::string subcategory = UI::instance().dialog("Choose subcategory", subcategories);
		if (subcategory.empty()) return;
		const nlohmann::json filter = {{"subcategory", subcategory}};
		std::vector<std::string> entity_names = EntityFactory::instance().filter_entity_ids(filter, SIZE_MAX);
		std::sort(entity_names.begin(), entity_names.end());
		const std::string entity_name = UI::instance().dialog("Choose entity", entity_names);
		if (entity_name.empty()) return;
		EntityFactory::instance().create_entity(registry, entity_name, registry.get<Position>(ECS::get_player(registry)));
	}

	void dev_menu(entt::registry& registry)
	{
		const std::vector<std::string> choices =
		{
			"God mode",
			"Show entities data",
			"Show/hide debug",
			"Change liquid",
			"Spawn liquid",
			"Spawn entity"
		};
		const auto& choice = UI::instance().dialog("DevTools", choices);
		if (choice == "God mode")
		{
			registry.ctx().get<Dev>().god_mode ^= true;
		}
		else if (choice == "Show entities data")
			show_entities(registry);
		else if (choice == "Show/hide debug")
			registry.ctx().get<Dev>().show_debug ^= true;
		else if (choice == "Change liquid")
			change_liquid(registry);
		else if (choice == "Spawn liquid")
		{
			//spawn_liquid();
		}
		else if (choice == "Spawn entity")
			spawn_entity(registry);
	}
};
