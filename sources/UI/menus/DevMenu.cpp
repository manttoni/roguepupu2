#include <stddef.h>
#include <stdint.h>
#include <nlohmann/detail/json_ref.hpp>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <vector>
#include <string>
#include <algorithm>
#include <map>
#include <optional>
#include <utility>

#include "UI/Dialog.hpp"
#include "UI/Menu.hpp"
#include "UI/menus/DevMenu.hpp"
#include "components/Components.hpp"
#include "database/EntityFactory.hpp"
#include "domain/Liquid.hpp"
#include "domain/Position.hpp"
#include "external/entt/entity/fwd.hpp"
#include "external/entt/entt.hpp"
#include "infrastructure/DevSettings.hpp"
#include "utils/ECS.hpp"

namespace DevTools
{
	void show_settings_menu(entt::registry& registry)
	{
		using Element = Menu::Element;
		using Type = Element::Type;

		Menu menu;
		menu.add_element(Element::text("Dev Settings"));
		menu.add_element(Element::line());
		for (auto& [type, setting] : registry.ctx().get<DevSettings>().settings)
		{
			if (auto p = std::get_if<bool>(&setting.value))
			{
				Element e(Type::Checkbox, setting.label, p);
				menu.add_element(e);
			}
		}
		menu.add_element(Element::confirm());
		menu.get_selection();
	}

	void dev_menu(entt::registry& registry)
	{
		Menu::Selection selection;
		while (true)
		{
			selection = Dialog::get_selection("Dev Tools", {"Settings", "Back"}, Screen::middle(), selection.index);
			if (selection.cancelled)
				break;
			if (selection.element->label == "Settings")
				show_settings_menu(registry);
		}
	}
};
