#include <filesystem>
#include <ncurses.h>
#include <nlohmann/json.hpp>
#include <panel.h>
#include <string>
#include <vector>
#include <optional>

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

	// Save all entities in this file
	static const std::filesystem::path entity_file = "data/entities.json";

	// This file has all components organized under tags. Add tags in editor and it will automatically add the components
	static const std::filesystem::path tag_dependencies_file = "data/components/tag_dependencies.json";

	void print_json(const Json& json)
	{
		// Prevent printing when nothing has changed
		static Json copy;
		if (copy != json)
			copy = json;
		else
			return;

		UI::instance().set_current_panel(UI::Panel::Editor);
		UI::instance().clear();
		UI::instance().print(0, 0, json.dump(4));
		// updating happens all the time in Menu
	}

	/* json already has update, but that one replaces and modifies original
	 * */
	Json merge_objects(const Json& a, const Json& b, const bool replace = false)
	{
		if (a.type() != b.type() || !a.is_object())
			return Json{};
		Json merged = a;
		for (const auto& [key, value] : b.items())
		{
			if (merged.contains(key) && !replace)
				continue;
			merged[key] = value;
		}
		return merged;
	}

	/* Take entries from data object for each key in keys array,
	 * and form a new Json from them
	 * */
	Json create_from_keys(const Json& data, const Json& keys)
	{
		if (!keys.is_array() || !data.is_object())
			return Json{};
		Json result = Json::object();
		for (const auto& json_key : keys)
		{
			const auto key = json_key.get<std::string>();
			if (data.contains(key))
				result[key] = data[key];
		}
		return result;
	}

	/* Add all array elements from template_array not present in target, no overwriting.
	 * */
	void add_missing_array(Json& target, const Json& template_array, std::vector<std::string>* errors = nullptr)
	{
		if (!target.is_array() || !template_array.is_array())
			return;

		for (const auto& e : template_array)
		{
			if (std::find(target.begin(), target.end(), e) == target.end())
			{
				if (errors) errors->push_back("[missing element in array]");
				target.push_back(e);
			}
		}
	}

	/* Template json holds entries that should be present in target.
	 * If they are not, adds default values except for strings if they are too short.
	 * If given errors vector, add error strings there.
	 * There should be some documentation about template_json format somewhere at some point.
	 * But basically it can have string min_length, or numbers min/max/delta/default,
	 * can also be "multi_choice": ["array", "of", "choices"]
	 * */
	void add_missing(Json& target, const Json& template_json, std::vector<std::string>* errors = nullptr)
	{
		for (const auto& [key, value] : template_json.items())
		{
			if (value.contains("range"))
			{
				if (!JsonUtils::is_range(value))
					Error::fatal("Invalid template json: unexpected 'range'");
				if (!target.contains(key))
				{
					if (errors) errors->push_back("[missing range: " + key + "]");
					target[key] = Json::object();
					target[key]["range"] = value["default"];
				}
				else if (!target[key].is_object())
				{
					if (errors) errors->push_back("[mistyped range: " + key + "]");
					target[key] = Json::object();
					target[key]["range"] = value["default"];
				}
				else
				{
					const Range<double> r = Parser::parse_range<double>(target[key]["range"]);
					const Range<double> r_limits = Parser::parse_range<double>(value["range"]);
					if (!r_limits.contains(r.min) || !r_limits.contains(r.max))
					{
						if (errors) errors->push_back("[misranged range: " + key + "]");
						target[key]["range"][0] = Math::clamp(r.min, r_limits.min, r_limits.max);
						target[key]["range"][1] = Math::clamp(r.max, r_limits.min, r_limits.max);
					}
				}
			}
			else if (value.contains("min") || value.contains("max"))
			{
				if (!value.contains("min") || !value.contains("max"))
					Error::fatal("Unexpected 'min' or 'max' in json value (need both): " + value.dump(4));
				if (!target.contains(key))
				{
					if (errors) errors->push_back("[missing number: " + key + "]");
					target[key] = value["default"];
				}
				else if (target[key].type_name() != value["min"].type_name())
				{
					if (errors) errors->push_back("[mistyped: " + key + ", should be " + std::string(value["min"].type_name()) + ", but is " + std::string(target[key].type_name()) + "]");
					target[key] = value["default"];
				}
				else if (target[key] < value["min"] || target[key] > value["max"])
				{
					if (errors) errors->push_back("[misranged: " + key + "]");
					target[key] = Math::clamp(target[key], value["min"], value["max"]);
				}
			}
			else if (value.contains("min_length"))
			{
				if (!target.contains(key))
				{
					if (errors) errors->push_back("[missing string: " + key + "]");
					target[key] = "";
				}
				else if (!target[key].is_string())
				{
					if (errors) errors->push_back("[not a string: " + key + "]");
					target[key] = "";
				}
				else if (target[key].get<std::string>().size() < value["min_length"].get<size_t>())
				{
					if (errors) errors->push_back("[string too short: " + key + "]");
				}
			}
			else if (value.is_object() && value.contains("multi_choice") && value["multi_choice"].is_array())
			{
				if (!target.contains(key))
				{
					if (errors) errors->push_back("[missing multi_choice: " + key + "]");
					target[key] = value["multi_choice"][0];
				}
			}
			else if (value.is_object())
			{
				if (!target.contains(key))
				{
					if (errors) errors->push_back("[missing object: " + key + "]");
					target[key] = Json::object();
				}
				add_missing(target[key], value, errors);
			}
			else if (value.is_array())
			{
				if (!target.contains(key))
				{
					if (errors) errors->push_back("[missing array: " + key + "]");
					target[key] = Json::array();
				}
				add_missing_array(target, value, errors);
			}
			else
			{
				if (!target.contains(key))
				{
					if (errors)
						errors->push_back("[missing " + std::string(value.type_name()) + ": " + key + "]");
					target[key] = value;
				}
			}
		}
	}

	/* When removing a tag, it will call update_entity, which will call this to remove components
	 * that are redundant without that tag. No need to do recursion into data, just remove on the component level
	 * */
	void erase_redundant(Json& entity, const Json& template_json, std::vector<std::string>* errors = nullptr)
	{
		for (auto it = entity.begin(); it != entity.end(); )
		{
			const auto& key = it.key();
			if (!template_json.contains(key))
			{
				if (errors) errors->push_back("[redundant component: " + key + "]");
				it = entity.erase(it);
			}
			else
				it++;
		}
	}

	/* template_json will be a set of info about components and how they should be like
	 * */
	Json get_template_json(const Json& entity, const bool get_full_template = false)
	{
		auto tag_deps = Parser::read_json_file(tag_dependencies_file);

		if (get_full_template == false)
			tag_deps = create_from_keys(tag_deps, entity["tags"]);

		Json template_json = Json::object();

		for (const auto& [tag, components] : tag_deps.items())
		{
			for (const auto& [component, data] : components.items())
				template_json[component] = data;
		}

		return template_json;
	}

	/* Aka auto-correct. Give default values.
	 * */
	void update_entity(Json& entity)
	{
		const auto template_json = get_template_json(entity);

		// If entity has a tag that depends on some component, add them
		add_missing(entity, template_json);

		// Same but erase if tag was removed
		erase_redundant(entity, template_json);
	}

	/* Check if add_missing/auto-correct would change something if it had the chance.
	 * Give vector to know what it would change.
	 * */
	bool verify_entity(const Json& entity, std::vector<std::string>* errors = nullptr)
	{
		const auto template_json = get_template_json(entity);
		Json copy = entity;
		std::vector<std::string> dummy;
		if (errors == nullptr)
			errors = &dummy;
		add_missing(copy, template_json, errors);
		erase_redundant(copy, template_json, errors);
		const bool no_modifications = entity == copy;
		const bool no_errors = errors == nullptr || errors->empty();
		return no_modifications && no_errors;
	}

	/* Find entity from file by name
	 * */
	Json load_entity(const std::string& name)
	{
		const Json entities = Parser::read_json_file(entity_file);
		for (const auto& entity : entities)
		{
			if (entity["name"].get<std::string>() == name)
				return entity;
		}
		return Json{};
	}

	/* Find from file by user selection
	 * */
	Json load_entity()
	{
		const Json entities = Parser::read_json_file(entity_file);

		std::string search_word = "";
		bool show_valid = true;
		bool show_invalid = true;
		Selection selection;
		while (true)
		{
			Menu menu;
			menu.set_blocking(false);

			Element search(Type::TextField, "Search", &search_word);
			menu.add_element(search);

			Element toggle_valid(Type::Checkbox, "Show valid", &show_valid);
			menu.add_element(toggle_valid);

			Element toggle_invalid(Type::Checkbox, "Show invalid", &show_invalid);
			menu.add_element(toggle_invalid);

			menu.add_element(Element::cancel());

			Json subset = Json::array();
			for (const auto& entity : entities)
			{
				const auto name = entity["name"].get<std::string>();
				bool is_valid = verify_entity(entity);
				if (name.find(search_word) == std::string::npos)
					continue;
				if (is_valid && !show_valid)
					continue;
				if (!is_valid && !show_invalid)
					continue;
				Element e(Type::Button, name);
				e.highlight = is_valid ? 'O' : 'X';
				menu.add_element(e);
				subset.push_back(entity);
			}
			print_json(subset);
			selection = menu.get_selection(selection.index);
			if (selection.cancelled)
				break;
			if (selection.timed_out)
				continue;

			assert(selection.element.has_value());
			for (const auto& entity : entities)
			{
				if (selection.element->label == entity["name"].get<std::string>())
					return entity;
			}
			assert(0 || "Something is wrong");
		}
		return Json{};
	}

	/* Get a blank entity with only "core" tag and default core components
	 * */
	Json new_entity()
	{
		Json entity;
		entity["tags"] = Json::array();
		entity["tags"].push_back("core");
		update_entity(entity);
		return entity;
	}

	/* Write entity to file
	 * */
	void save_entity(const Json& entity)
	{
		if (!verify_entity(entity))
		{
			Dialog::alert("Cannot save invalid entity");
			return;
		}

		Json data = Parser::read_json_file(entity_file);
		bool exists = false;
		for (auto& entry : data)
		{
			if (entry["name"] == entity["name"])
			{
				exists = true;
				if (!Dialog::confirm("\"" + entity["name"].get<std::string>() + "\" already exists. Overwrite?"))
					return;
				else
				{
					entry = entity;
					break;
				}
			}
		}
		if (exists == false)
			data.push_back(entity);
		std::ofstream file(entity_file);
		assert(file);
		file << data.dump(4);
		Dialog::alert("Entity saved");
	}

	/* Add or remove tags. When edited, update entity so it has necessary components, or remove unnecessary ones
	 * */
	void edit_tags(Json& entity)
	{
		Json& tags = entity["tags"];
		assert(tags.is_array());

		// for list of tags
		const auto tag_dependencies = Parser::read_json_file(tag_dependencies_file);
		Selection selection;
		while (true)
		{
			print_json(entity);

			Menu menu(Screen::topright());
			menu.set_blocking(false);
			menu.add_element(Element::text("Add/remove tags"));
			menu.add_element(Element::line());

			for (const auto& [tag, deps] : tag_dependencies.items())
			{
				Element e(Type::Button, tag);
				if (std::find(tags.begin(), tags.end(), tag) != tags.end())
					e.highlight = L'*';
				menu.add_element(e);
			}

			menu.add_element(Element::confirm());
			selection = menu.get_selection(selection.index);
			if (selection.cancelled || selection.confirmed)
				break;
			if (selection.timed_out)
				continue;

			assert(selection.element.has_value());
			const auto& tag = selection.element->label;
			if (tag == "core")
			{
				Dialog::alert("Don't remove core!");
				return;
			}
			auto it = std::find(tags.begin(), tags.end(), tag);
			if (it == tags.end())
				tags.push_back(tag);
			else
				tags.erase(it);
			update_entity(entity);
		}
	}

	/* 'entity' is the entity being edited, edit it through 'part'
	 * 'part' is the part of entity being edited
	 * 'template_json' is what 'part' should be like: range, type, keys.
	 * In this function 'template_json' can have every possible component,
	 * but when calling 'verify_entity' and 'update_entity', they will form their own sub template
	 * */
	void edit_entity(const Json& entity, Json& part, const Json& template_json, const std::string& part_id = "")
	{
		Log::log("Editing entity: " + entity.dump(4));
		Log::log("Editing part: " + part.dump(4));
		Log::log("Template: " + template_json.dump(4));
		Selection selection;
		while (true)
		{
			print_json(entity);

			Menu menu(Screen::topright());
			menu.set_blocking(false);

			// Show entity name and maybe other stuff too
			const std::string name = entity["name"].get<std::string>();
			const Color color = Parser::parse_color(entity["color"]);
			std::vector<std::string> errors;
			const std::string head = color.markup() + name + "{reset} " + (verify_entity(entity, &errors) ? "🟢" : "🔴");
			menu.add_element(Element(Type::Text, head));
			for (const auto& error : errors)
				menu.add_element(Element::text(error));
			menu.add_element(Element::line());

			if (!part_id.empty())
			{
				menu.add_element(Element::text(part_id));
				menu.add_element(Element::line());
			}

			// Add Menu::Elements for values in 'part'
			for (auto& [key, value] : part.items())
			{
				if (!template_json.contains(key))
					continue;
				Log::log("Creating Menu::Element for key: " + key);
				Type type = Type::None;

				if (value.is_number())
				{
					type = Type::ValueSelector;
					const Range<int> range(
								template_json[key]["min"].get<int>(),
								template_json[key]["max"].get<int>()
								);
					const double delta = template_json[key]["delta"].get<double>();
					menu.add_element(Element(
							Type::ValueSelector,
							key,
							&value,
							range,
							delta
							));
					continue;
				}
				else if (JsonUtils::is_range(value) && JsonUtils::is_range(template_json[key])) // { "range": [min, max] } both in value and template[key]
				{
					const Range<int> range(
							template_json[key]["range"][0].get<int>(),
							template_json[key]["range"][1].get<int>()
							);
					const double delta = template_json[key]["delta"].get<double>();
					Element e(
							Type::RangeSelector,
							key,
							&value,
							range,
							delta
							);
					menu.add_element(e);
					continue;
				}
				else if (value.is_string() && template_json[key].is_object() && template_json[key].contains("multi_choice"))
				{
					Element e(
							Type::MultiChoice,
							key,
							&value,
							template_json[key]["multi_choice"].get<std::vector<std::string>>()
							);
					menu.add_element(e);
					continue;
				}
				else if (value.is_object() || value.is_array())
				{
					menu.add_element(Element(Type::Button, key));
					continue;
				}
				else if (value.is_string())
				{
					menu.add_element(Element(Type::TextField, key, &value, Range<int>(0, 20)));
					continue;
				}
				else if (value.is_boolean())
					type = Type::Checkbox;
				else
					Error::fatal("Unhandled json type: " + std::string(value.type_name()) + value.dump(4));

				menu.add_element(Element(type, key, &value));
			}

			if (entity == part)
			{
				menu.add_element(Element(Type::Button, "Auto correct"));
				menu.add_element(Menu::Element::confirm("Save"));
			}
			menu.add_element(Menu::Element::cancel());

			selection = menu.get_selection(selection.index);
			if (selection.cancelled)
			{
				break;
			}
			else if (selection.confirmed)
			{
				save_entity(entity);
				continue;
			}
			else if (selection.timed_out)
				continue;

			assert(selection.element.has_value());

			const auto& label = selection.element->label;
			if (label == "tags")
				edit_tags(part);
			else if (label == "Auto correct")
				update_entity(part);
			else
			{
				assert(part.contains(label));
				assert(template_json.contains(label));
				assert(part[label].is_object() || part[label].is_array());
				edit_entity(entity, part[label], template_json[label], Utils::capitalize(label));
			}
		}
	}

	void entity_editor()
	{
		static const std::vector<std::string> buttons = {
			"New Entity", "Load Entity", "Back"
		};
		Selection selection;
		while (true)
		{
			Json entity;
			selection = Dialog::get_selection("** Editor **", buttons, Screen::middle(), selection.index);
			if (selection.cancelled)
				break;

			assert(selection.element.has_value());
			const auto label = selection.element->label;
			if (label == "New Entity")
				entity = new_entity();
			else if (label == "Load Entity")
			{
				entity = load_entity();
				if (entity == Json{})
					continue;
			}

			// true = get full template, sub template is not enough because tags change
			const auto template_json = get_template_json(entity, true);
			edit_entity(entity, entity, template_json);
		}
	}

	void start()
	{
		UI::instance().set_current_panel(UI::Panel::Editor, true);
		static const std::vector<std::string> buttons = {
			"Entity Editor", "Quit"
		};
		Selection selection;
		while (true)
		{
			selection = Dialog::get_selection("** Editor **", buttons, Screen::middle(), selection.index);
			if (selection.cancelled)
				break;

			assert(selection.element.has_value());
			if (selection.element->label == "Entity Editor")
				entity_editor();
		}
	}
};
