#include <vector>
#include <string>
#include "DevTools.hpp"
#include "UI.hpp"
#include "entt.hpp"
#include "ECS.hpp"
#include "Cave.hpp"
#include "Cell.hpp"

namespace DevTools
{
	void dev_menu(entt::registry& registry)
	{
		const std::vector<std::string> data =
		{
			"Show stuff here"
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
