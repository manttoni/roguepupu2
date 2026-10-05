#include <sstream>
#include "ui/Subset.hpp"
#include "ui/Search.hpp"
#include "editor/EntityEditor.hpp"
#include "ui/Theme.hpp"
#include "utils/Log.hpp"
#include "utils/IO.hpp"
#include "ui/Dialog.hpp"
#include "ui/Menu.hpp"
#include "ui/Element.hpp"
#include "ncurses/Color.hpp"


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

	std::optional<Definition> search_definition(const Json& all_definitions)
	{
		UI::Menu menu("Search Definition");
		for (const auto& [id, data] : all_definitions.items())
			menu.add(UI::Element::Button(id));
		menu.add(UI::Element::cancel());
		const auto selection = UI::Search::select(menu);
		if (selection.cancelled())
			return std::nullopt;
		return Definition{.id = selection.label, .data = all_definitions.at(selection.label)};
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
		std::string id;
		const auto selection = UI::Dialog::get_input("Entity ID", id);
		if (selection.cancelled())
			return std::nullopt;
		assert(selection.confirmed());

		if (!Game::Entity::valid_id(id))
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
	 * Save 'active' into 'all_definitions' if user confirms. Still needs to be written to file.
	 * */
	void activate_definition(Json& all_definitions, Definition& active, const std::optional<Definition>& definition)
	{
		if (!definition.has_value())
			return;
		if (!definition_matches(all_definitions, active) && !active.id.empty())
		{
			if (UI::Dialog::get_selection("Save \"" + active.id + "\"?", {"Yes", "No"}).label == "Yes")
				add_definition(all_definitions, active);
		}
		active = *definition;
	}

	void print_definition(Ncurses::Window& surface, const Definition& definition)
	{
		surface.clear();
		const std::string str = "\"" + definition.id + "\": " + definition.data.dump(4);
		surface.write(0, 0, str);
	}
	template<typename T>
		Json value_to_json(const T& value)
		{
			if constexpr (std::same_as<T, char>)
				return std::string(1, value);
			else if constexpr (std::same_as<T, Ncurses::Color>)
				return value.id();
			else if constexpr (Game::Enum::GameEnum<T>)
				return Game::Enum::to_string(value);
			else
				return Json(value);
		}
	Json component_default(std::string_view id)
	{
#define X(name, type) \
		if (id == #name) \
		return value_to_json(Game::Component::Value::name{}.value);
#include "game/components/Value.def"
#undef X

#define X(name, type) \
		if (id == #name) \
		return value_to_json(Game::Component::Resource::name{}.maximum);
#include "game/components/Resource.def"
#undef X

#define X(name, type) \
		if (id == #name) \
		return Json::array();
#include "game/components/List.def"
#undef X

		throw std::invalid_argument(
				"Unknown component: " + std::string(id));
	}

	/* Tags can require other Components
	 * */
	bool update_dependencies(Definition& definition)
	{
		auto& data = definition.data;

		if (!data.contains("Tags"))
			data["Tags"] = Json::array();

		const auto tags =
			data.at("Tags").get<std::vector<std::string>>();

		bool changed = false;

		for (const auto& tag : tags)
		{
			const auto tag_deps =
				Game::Component::Dependency::get_required_tag_ids(tag);

			for (const auto& dependency : tag_deps)
			{
				auto& current_tags = data.at("Tags");

				if (std::find(
							current_tags.begin(),
							current_tags.end(),
							dependency) == current_tags.end())
				{
					current_tags.push_back(dependency);
					changed = true;
				}
			}

			const auto other_deps =
				Game::Component::Dependency::get_required_non_tag_ids(tag);

			for (const auto& dependency : other_deps)
			{
				if (!data.contains(dependency))
				{
					data[dependency] = component_default(dependency);
					changed = true;
				}
			}
		}

		return changed;
	}

	void edit_tags(Definition& definition)
	{
		const auto all_tags = Game::Component::get_tag_ids();
		auto tags = definition.data["Tags"].get<std::vector<std::string>>();
		UI::Subset::edit(all_tags, tags);
		definition.data["Tags"] = tags;
		while (update_dependencies(definition)) {}
	}

	void build_edit_definition_menu(UI::Menu& menu, Definition& definition)
	{
		Log::debug() << "Building menu menu";

		menu.set_title("Editing \'" + definition.id + "\'");
		menu.set_timeout(500);
		menu.clear_elements();

		for (auto& [component_id, component_data] : definition.data.items())
		{
			if (component_id == "Color")
			{
				menu.add(UI::Element::ValueSelector<Json::number_integer_t>(
							component_id,
							component_data.get_ptr<Json::number_integer_t*>(),
							{16,231}
							));
			}
			else if (component_data.is_boolean())
			{
				menu.add(UI::Element::Checkbox(
							component_id,
							component_data.get_ptr<bool*>()
							));
			}
			else if (component_data.is_string())
			{
				if (Game::Component::value_is_enum(component_id))
				{
					menu.add(UI::Element::SingleChoice(
								component_id,
								component_data.get_ptr<std::string*>(),
								Game::Component::get_enum_value_strings(component_id)
								));
					continue;
				}
				menu.add(UI::Element::TextIn(
							component_id,
							component_data.get_ptr<std::string*>()
							));
			}
			else if (component_data.is_number_unsigned())
			{
				using T = Json::number_unsigned_t;

				menu.add(UI::Element::ValueSelector<T>{
						component_id,
						component_data.get_ptr<T*>(),
						{0, 100}
						});
			}
			else if (component_data.is_number_integer())
			{
				using T = Json::number_integer_t;

				menu.add(UI::Element::ValueSelector<T>{
						component_id,
						component_data.get_ptr<T*>(),
						{-100, 100}
						});
			}
			else if (component_data.is_number_float())
			{
				using T = Json::number_float_t;

				menu.add(UI::Element::ValueSelector<T>{
						component_id,
						component_data.get_ptr<T*>(),
						{-100, 100}
						});
			}
			else if (component_data.is_array())
			{
				menu.add(UI::Element::MultiChoice{
						component_id,
						component_data.size()
						});
			}
		}

		menu.add(UI::Element::confirm());
		menu.add(UI::Element::cancel());
	}

	void edit_definition(Definition& definition)
	{
		const auto original = definition;

		while (true)
		{
			UI::Menu menu;
			build_edit_definition_menu(menu, definition);
			const auto selection = UI::Search::select(menu);
			switch (selection.state)
			{
				case UI::Selection::State::Cancelled:
					definition = original;
					return;
				case UI::Selection::State::Confirmed:
					return;
				case UI::Selection::State::MultiChoice:
					if (selection.label == "Tags")
						edit_tags(definition);
					continue;
				default:
					Log::error() << selection;
					return;
			}
		}
	}

	void start()
	{
		Definition active;
		Json all_definitions = IO::read_json(EntityDatabase::entity_definitions_path);
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
				all_definitions = IO::read_json(EntityDatabase::entity_definitions_path);
				UI::Dialog::alert("Definitions read, size: " + std::to_string(all_definitions.size()));
			}
			else if (label == "Write")
			{
				if (IO::write_json(EntityDatabase::entity_definitions_path, all_definitions) == true)
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
				throw std::runtime_error("Unexpected label: " + label);
		}
	}
}
