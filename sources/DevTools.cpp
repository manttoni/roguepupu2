#include <vector>
#include <string>
#include "EntityFactory.hpp"
#include "DevTools.hpp"
#include "UI.hpp"
#include "entt.hpp"
#include "ECS.hpp"
#include "Cave.hpp"
#include "Cell.hpp"
#include "Components.hpp"
#include "Color.hpp"

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

	void show_elevation(const entt::registry& registry)
	{
		Cave* cave = ECS::get_cave(registry, ECS::get_player(registry));
		auto& cells = cave->get_cells();
		static bool showing = false;
		if (showing == false)
		{
			for (auto& cell : cells)
			{
				const double density = cell.get_density();
				const size_t digit = static_cast<size_t>(std::round(density * -10.0));
				if (digit > 0 && digit < 10)
				{
					cell.set_fgcolor(Color(500, 500, 500));
					cell.set_glyph(L'0' + digit);
				}
			}
		}
		else
		{
			for (auto& cell : cells)
			{
				cell.set_fgcolor(Color(35, 40, 30));
				cell.set_glyph(L' ');
			}
		}
		showing ^= true;
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
		const std::vector<std::string> subcategories = EntityFactory::instance().get_subcategory_names(category);
		const std::string subcategory = UI::instance().dialog("Choose subcategory", subcategories);
		const nlohmann::json filter = {{"subcategory", subcategory}};
		std::vector<std::string> entity_names = EntityFactory::instance().random_pool(filter, SIZE_MAX);
		std::sort(entity_names.begin(), entity_names.end());
		const std::string entity_name = UI::instance().dialog("Choose entity", entity_names);
		Cell* cell = ECS::get_cell(registry, ECS::get_player(registry));
		EntityFactory::instance().create_entity(registry, entity_name, cell);
	}

	void dev_menu(entt::registry& registry)
	{
		const std::vector<std::string> choices =
		{
			"God mode",
			"Reveal/hide map",
			"Show entities data",
			"Show/hide debug",
			"Show/hide elevation",
			"Change liquid",
			"Spawn liquid",
			"Spawn entity"
		};
		const auto& choice = UI::instance().dialog("DevTools", choices);
		if (choice == "God mode")
		{
			registry.ctx().get<Dev>().god_mode ^= true;
		}
		if (choice == "Reveal/hide map")
		{
			static bool hidden = true;
			Cave* cave = ECS::get_active_cave(registry);
			auto& cells = cave->get_cells();
			for (auto& cell : cells)
				cell.set_seen(hidden);
			hidden = !hidden;
		}
		else if (choice == "Show entities data")
			show_entities(registry);
		else if (choice == "Show/hide debug")
			registry.ctx().get<Dev>().show_debug ^= true;
		else if (choice == "Show/hide elevation")
			show_elevation(registry);
		else if (choice == "Change liquid")
			change_liquid(registry);
		else if (choice == "Spawn liquid")
		{
			static size_t multi = 1;
			ECS::get_cell(registry, ECS::get_player(registry))->get_liquid_mixture().add_liquid(registry.ctx().get<Dev>().liquid_type, 1 * multi++);
		}
		else if (choice == "Spawn entity")
			spawn_entity(registry);
	}
};
