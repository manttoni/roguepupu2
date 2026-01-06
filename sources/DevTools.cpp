#include <vector>
#include <string>
#include "DevTools.hpp"
#include "UI.hpp"
#include "entt.hpp"
#include "ECS.hpp"
#include "Cave.hpp"
#include "Cell.hpp"
#include "Components.hpp"

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

	void dev_menu(entt::registry& registry)
	{
		const std::vector<std::string> choices =
		{
			"God mode",
			"Reveal/hide map",
			"Show entities data",
			"Show/hide debug"
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
	}
};
