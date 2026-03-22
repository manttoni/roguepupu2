#include <filesystem>
#include <ncurses.h>
#include <nlohmann/json.hpp>
#include <panel.h>
#include <string>
#include <vector>
#include <optional>

#include "editor/CaveEditor.hpp"
#include "editor/EntityEditor.hpp"
#include "components/Components.hpp"
#include "UI/Dialog.hpp"
#include "UI/Menu.hpp"
#include "UI/UI.hpp"
#include "editor/Editor.hpp"
#include "utils/Error.hpp"
#include "utils/Parser.hpp"
#include "utils/Screen.hpp"
#include "utils/Range.hpp"
#include "utils/Math.hpp"
#include "utils/Utils.hpp"
#include "utils/JsonUtils.hpp"

namespace Editor
{
	using Element = Menu::Element;
	using Type = Element::Type;
	using Selection = Menu::Selection;
	using Json = nlohmann::json;

	void print_json(const Json& json, const size_t size)
	{
		// Prevent printing when nothing has changed
		static Json copy;
		if (copy != json)
			copy = json;
		else
			return;

		UI::instance().set_current_panel(UI::Panel::Editor);
		UI::instance().clear();
		UI::instance().print(0, 0, json.dump(size));
	}

	void start()
	{
		UI::instance().set_current_panel(UI::Panel::Editor, true);
		static const std::vector<std::string> buttons = {
			"Entity Editor", "Cave Editor", "Loot Table Editor", "Quit"
		};
		Selection selection;
		while (true)
		{
			selection = Dialog::get_selection("** Editor **", buttons, Screen::middle(), selection.index);
			if (selection.cancelled)
				break;

			assert(selection.element.has_value());
			if (selection.element->label == "Entity Editor")
				EntityEditor::entity_editor();
		}
	}
};
