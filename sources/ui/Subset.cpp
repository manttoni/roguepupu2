#include "ui/Element.hpp"
#include "ui/Menu.hpp"
#include "ui/Search.hpp"
#include "ui/Subset.hpp"
#include <stdexcept>
#include <algorithm>
#include <map>

namespace UI::Subset
{
	Selection edit(const std::vector<std::string>& all, std::vector<std::string>& subset)
	{
		const auto original = subset;
		UI::Menu menu;
		std::map<std::string, bool> selected;
		for (const auto& s : all)
		{
			bool in_subset =
				std::find(subset.begin(), subset.end(), s) != subset.end();
			selected[s] = in_subset;
			menu.add(UI::Element::Checkbox(s, &selected[s]));
		}
		menu.add(UI::Element::confirm());
		menu.add(UI::Element::cancel());
		const auto selection = UI::Search::select(menu);
		if (selection.cancelled())
		{
			subset = original;
		}
		else if (selection.confirmed())
		{
			subset.clear();
			for (const auto& [s, b] : selected)
			{
				if (b)
					subset.push_back(s);
			}
		}
		else
			throw std::runtime_error(
					std::string("Unexpected selection state: ")
					+ std::string(to_string(selection.state)));

		return selection;
	}
}
