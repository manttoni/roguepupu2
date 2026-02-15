#include "UI/menus/SettingsMenu.hpp"
#include "UI/Menu.hpp"
#include "infrastructure/GameSettings.hpp"
#include "external/entt/entt.hpp"

namespace SettingsMenu
{
	void show_menu(entt::registry& registry)
	{
		using Element = Menu::Element;
		using Type = Element::Type;
		Menu menu;
		menu.add_element(Element(Type::Text, "Settings"));
		menu.add_element(Element(Type::Text, "--"));
		for (auto& [type, setting] : registry.ctx().get<GameSettings>().settings)
		{
			if (bool* pb_val = std::get_if<bool>(&setting.value))
			{
				Menu::Element::ElementValue val = pb_val;
				menu.add_element(Element(Menu::Element::Type::Checkbox, setting.label, val));
			}
			else if (int* pi_val = std::get_if<int>(&setting.value))
			{
				Menu::Element::ElementValue val = pi_val;
				if (type == GameSettings::Type::LogLength)
					menu.add_element(Element(Menu::Element::Type::ValueSelector, setting.label, val, 0, Screen::height()));
			}
		}
		menu.add_element(Element(Type::Button, "Apply"));
		menu.get_selection();
	}
};
