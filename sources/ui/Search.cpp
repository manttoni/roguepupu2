#include "ui/Search.hpp"

namespace UI::Search
{
	UI::Selection select(const UI::Menu& menu)
	{
		Menu search_menu(menu.get_title());
		search_menu.set_timeout(100);

		std::string needle;

		Element::TextIn search_element("Search", &needle);
		const auto& elements = menu.get_elements();

		Selection selection;
		while (true)
		{
			if (search_menu.changed())
			{
				search_menu.clear_elements();
				search_menu.add(search_element);
				for (const auto& e : elements)
				{
					const bool has_special_role =
						std::visit([](const auto& c)
							{
								if constexpr (requires {c.role;})
									return c.role != Element::Role::Normal;
								return false;
							}
							, e);
					if (get_label(e).find(needle) != std::string::npos
							|| has_special_role)
						search_menu.add(e);
				}
			}
			selection = search_menu.get_selection(selection.index);
			if (selection.timed_out())
				continue;
			return selection;
		}
	}
}
