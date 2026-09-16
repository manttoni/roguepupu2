#include "editor/EntityEditor.hpp"
#include "ui/Theme.hpp"
#include "utils/Error.hpp"
#include "utils/IO.hpp"
#include "ui/Dialog.hpp"
#include "ui/Menu.hpp"
#include "ui/Element.hpp"

namespace EntityEditor
{
	bool erase_definition(Json& all_definitions, const Definition& definition)
	{
		if (!all_definitions.contains(definition.id))
			return false;
		all_definitions.erase(definition.id);
		return true;
	}

	bool add_definition(Json& all_definitions, const Definition& definition)
	{
		all_definitions[definition.id] = definition.data;
		return true;
	}

	std::vector<Definition> filter_definitions(const Json& all_definitions, const std::string& filter)
	{
		std::vector<Definition> filtered;

		for (const auto& [id, data] : all_definitions.items())
		{
			bool matches = filter.empty() || id.find(filter) != std::string::npos;

			if (!matches)
			{
				for (const auto& [component_id, component_data] : data.items())
				{
					if (component_data.is_string() && component_data.get<std::string>().find(filter) != std::string::npos)
					{
						matches = true;
						break;
					}
					if (component_id.find(filter) != std::string::npos)
					{
						matches = true;
						break;
					}
				}
			}

			if (matches)
				filtered.push_back({.id = id, .data = data});
		}

		return filtered;
	}

	std::optional<Definition> search_definition(const Json& all_definitions)
	{
		std::string filter = "";
		UI::Selection selection;
		while (true)
		{
			const auto filtered_definitions = filter_definitions(all_definitions, filter);
			UI::Menu search;
			search.set_title("Seach definition");
			search.set_timeout(100);
			search.add(UI::Element::TextIn("Filter", &filter));
			for (const auto& def : filtered_definitions)
			{
				search.add(UI::Element::Button(def.id));
			}
			search.add(UI::Element::cancel());

			selection = search.get_selection(selection.index);
			if (selection.selected())
				return Definition{.id = selection.label, .data = all_definitions.at(selection.label)};
			if (selection.cancelled())
				break;
		}
		return std::nullopt;
	}

	std::optional<Definition> load_definition(const Json& all_definitions)
	{
		const auto definition = search_definition(all_definitions);
		if (!definition.has_value())
			return std::nullopt;
		return definition;
	}

	std::optional<Definition> new_definition(const Json& all_definitions)
	{
		std::string id = "";
		UI::Menu new_entity;
		new_entity.set_title("New entity");
		new_entity.add(UI::Element::TextIn("Entity id", &id));
		new_entity.add(UI::Element::confirm());
		new_entity.get_selection();

		if (!Entity::valid_id(id))
			return std::nullopt;
		if (all_definitions.contains(id))
		{
			UI::Dialog::alert("Id already in use!");
			return std::nullopt;
		}
		Definition d{.id = id, .data = Json::object()};
		d.data["Tags"] = Json::array();


		return d;
	}

	/* Return true if 'definition' is already completely the same as one in 'all_definitions'
	 * */
	bool definition_matches(const Json& all_definitions, const Definition& definition)
	{
		return all_definitions.contains(definition.id) && all_definitions.at(definition.id) == definition.data;
	}

	/* Remove "active status" from active definition by giving it to 'definition' instead
	 * Save 'active' into 'all_definitions' if user confirms
	 * */
	void activate_definition(Json& all_definitions, Definition& active, const std::optional<Definition>& definition)
	{
		if (!definition.has_value())
			return;
		if (!definition_matches(all_definitions, active) && !active.id.empty())
		{
			if (UI::Dialog::get_selection("Save/overwrite \"" + active.id + "\"?", {"Yes", "No"}).label == "Yes")
				add_definition(all_definitions, active);
		}
		active = *definition;
	}

	void check_tags(Definition& definition)
	{
		const auto tags = definition.data["Tags"].get<std::vector<std::string>>();

	}

	void print_definition(Ncurses::Window& surface, const Definition& definition)
	{
		surface.clear();
		const std::string str = "\"" + definition.id + "\": " + definition.data.dump(4);
		surface.write(0, 0, str);
		surface.refresh();
	}

	void edit_definition(Definition& definition)
	{
		const auto original = definition;
		Ncurses::Panel edit_panel;
		edit_panel.get_window().enable_color(UI::load_theme().text);
		UI::Selection selection;
		while (true)
		{
			print_definition(edit_panel.get_window(), definition);
			UI::Menu editor;
			editor.set_title(definition.id +
					(Entity::valid_definition(definition.data) ? "" : "*")
					);
			editor.set_timeout(500);
			//editor.add(UI::Element::MultiChoice("Tags", &tags));

			for (auto& [component_id, component_data] : definition.data.items())
			{
				if (component_data.is_boolean())
				{
					editor.add(UI::Element::Checkbox(
								component_id,
								component_data.get_ptr<bool*>()
								));
				}
				else if (component_data.is_string())
				{
					if (Component::value_is_enum(component_id))
					{
						editor.add(UI::Element::SingleChoice(
									component_id,
									component_data.get_ptr<std::string*>(),
									Component::get_enum_value_strings(component_id)
									));
						continue;
					}
					editor.add(UI::Element::TextIn(
								component_id,
								component_data.get_ptr<std::string*>()
								));
				}
				else if (component_data.is_number_unsigned())
				{
					using T = Json::number_unsigned_t;

					editor.add(UI::Element::ValueSelector<T>{
							component_id,
							component_data.get_ptr<T*>(),
							{0, 100}
							});
				}
				else if (component_data.is_number_integer())
				{
					using T = Json::number_integer_t;

					editor.add(UI::Element::ValueSelector<T>{
							component_id,
							component_data.get_ptr<T*>(),
							{-100, 100}
							});
				}
				else if (component_data.is_number_float())
				{
					using T = Json::number_float_t;

					editor.add(UI::Element::ValueSelector<T>{
							component_id,
							component_data.get_ptr<T*>(),
							{-100, 100}
							});
				}
				/*else if (component_data.is_array())
				  {
				  editor.add(UI::Element::MultiChoice<std::string>(
				  component_id,
				  component_data.get_ptr<std::vector<std::string>*>()
				  ));
				  }*/
			}

			editor.add(UI::Element::confirm());	// apply changes
			editor.add(UI::Element::cancel());	// revert changes

			selection = editor.get_selection(selection.index);
			using State = UI::Selection::State;
			switch (selection.state)
			{
				case State::Pending:
					continue;
				case State::Selected:
					// should not happen, this is for buttons which are not confirm or cancel
					break;
				case State::Confirmed:
					return;
				case State::Cancelled:
					definition = original;
					return;
				case State::TimedOut:
					continue;
				case State::Error:
					Log::error() << "Error in edit_definition";
					continue;
				case State::SingleChoice:
					// shouldnt happen, or even exist, because Menu already handles editing SingleChoice
					continue;
				case State::MultiChoice:
					// opens another menu, where user can choose many, because Menu doesnt do that itself
					continue;
				case State::Ignored:
					// should not happen
					continue;
				case State::Changed:
					// should not happen
					continue;
			}
		}
	}

	void start()
	{
		Definition active;
		Json all_definitions = IO::read_json(IO::Paths::entities_file);
		UI::Selection selection;
		Ncurses::Panel back_panel;
		back_panel.get_window().enable_color(UI::load_theme().text);
		while (true)
		{
			print_definition(back_panel.get_window(), active);
			const auto active_id = (active.id.empty() ? "none" : active.id) + (definition_matches(all_definitions, active) ? "" : "*");
			UI::Menu editor;
			editor.set_title("Active entity: " + active_id);
			editor.add(UI::Element::Button("New"));		// Create a blank definition with a valid id, and set it active
			editor.add(UI::Element::Button("Load"));	// Load a definition from Json object, and set it active
			editor.add(UI::Element::Button("Edit"));	// Change values of active definition
			editor.add(UI::Element::Button("Add"));		// Add active definition to Json object
			editor.add(UI::Element::Button("Erase"));	// Erase active definition from Json object
			editor.add(UI::Element::Button("Read"));	// Read json object from file
			editor.add(UI::Element::Button("Write"));	// Write Json object to file
			editor.add(UI::Element::Button("Help"));	// Show help
			editor.add(UI::Element::Button("Quit"));	// Quit to main menu
			selection = editor.get_selection(selection.index);
			if (selection.cancelled())
				break;
			const auto& label = editor.get_label(selection.index);

			if (label == "New")
			{
				activate_definition(all_definitions, active, new_definition(all_definitions));
			}
			else if (label == "Load")
			{
				activate_definition(all_definitions, active, load_definition(all_definitions));
			}
			else if (label == "Edit")
			{
				edit_definition(active);
			}
			else if (label == "Add")
			{
				add_definition(all_definitions, active);
				UI::Dialog::alert("Definition added to json");
			}
			else if (label == "Erase")
			{
				erase_definition(all_definitions, active);
				active = Definition{};
				UI::Dialog::alert("Active definition erased from json");
			}
			else if (label == "Read")
			{
				all_definitions = IO::read_json(IO::Paths::entities_file);
				UI::Dialog::alert("Definitions read, size: " + std::to_string(all_definitions.size()));
			}
			else if (label == "Write")
			{
				if (IO::write_json(IO::Paths::entities_file, all_definitions) == true)
					UI::Dialog::alert("Write succesful");
				else
					UI::Dialog::alert("Write unsuccesful");
			}
			else if (label == "Help")
			{
				UI::Menu help;
				help.set_title("Help");
				help.add(UI::Element::Text("Activate entity with new/load."));
				help.add(UI::Element::Text("Then edit/add/erase."));
				help.add(UI::Element::Text("Write/Read to/from file."));
				help.add(UI::Element::Text("Title has '*' if does not match definition in file"));
				help.add(UI::Element::confirm());
				help.get_selection();
			}
			else if (label == "Quit")
			{
				break;
			}
			else
				Error::fatal("Unexpected label: " + label);
		}
	}
}
