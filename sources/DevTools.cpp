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

	void dev_menu(entt::registry& registry)
	{
		const std::vector<std::string> data =
		{
			"Registry has...",
			get_amount(registry, "creatures"),
			get_amount(registry, "mushrooms"),
			get_amount(registry, "plants")
		};
		const std::vector<std::string> choices =
		{
			"Reveal map"
		};
		const auto& choice = UI::instance().dialog(data, choices);
		if (choice == "Reveal map")
		{
			Cave* cave = ECS::get_active_cave(registry);
			auto& cells = cave->get_cells();
			for (auto& cell : cells)
				cell.set_seen(true);
		}
	}
};
