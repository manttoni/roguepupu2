#include <filesystem>
#include <ncurses.h>
#include <nlohmann/json.hpp>
#include <panel.h>
#include <string>
#include <vector>
#include <optional>

#include "core/paths.hpp"
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

namespace EntityEditor
{
	using Element = Menu::Element;
	using Type = Element::Type;
	using Selection = Menu::Selection;
	using Json = nlohmann::json;

	/* Form a new Json object from 'data'-object, but only with keys in 'keys'-array
	 * */
	Json create_from_keys(const Json& data, const Json& keys)
	{
		if (!keys.is_array() || !data.is_object())
			Error::fatal("Need data object and keys array");
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
			Error::fatal("Called with different types than array");

		for (const auto& e : template_array)
		{
			if (std::find(target.begin(), target.end(), e) == target.end())
			{
				if (errors)
				{
					std::string str = JsonUtils::to_string(e);
					errors->push_back(std::string("[missing ") + e.type_name() + " in array: " + str + "]");
				}
				target.push_back(e);
			}
		}
	}

	/* Template json holds component entries that should be present in target.
	 * If they are not, adds default values except for strings if they are too short.
	 * If given errors vector, add error strings there.
	 * There should be some documentation about template_json format somewhere at some point.
	 * But basically it can have string min_length, or numbers min/max/delta/default,
	 * can also be "template_multi_choice": ["array", "of", "choices"], i will definitely do a readme
	 * */
	void add_missing(Json& target, const Json& template_json, std::vector<std::string>* errors = nullptr)
	{
		for (const auto& [key, value] : template_json.items())
		{
			if (value.contains("template_range"))
			{
				if (!target.contains(key))
				{
					if (errors) errors->push_back("[missing range: " + key + "]");
					target[key] = Json::object();
					target[key]["range"] = value["template_default"];
				}
				else if (!target[key].is_object())
				{
					if (errors) errors->push_back("[mistyped range: " + key + "]");
					target[key] = Json::object();
					target[key]["range"] = value["template_default"];
				}
				else
				{
					const Range<double> r = Parser::parse_range<double>(target[key]["range"]);
					const Range<double> r_limits = Parser::parse_range<double>(value["template_range"]);
					if (!r_limits.contains(r.min) || !r_limits.contains(r.max))
					{
						if (errors) errors->push_back("[misranged range: " + key + "]");
						target[key]["range"][0] = Math::clamp(r.min, r_limits.min, r_limits.max);
						target[key]["range"][1] = Math::clamp(r.max, r_limits.min, r_limits.max);
					}
				}
			}
			else if (value.contains("template_min") || value.contains("template_max"))
			{
				if (!value.contains("template_min") || !value.contains("template_max"))
					Error::fatal("Unexpected 'min' or 'max' in json value (need both): " + value.dump(4));
				if (!target.contains(key))
				{
					if (errors) errors->push_back("[missing number: " + key + "]");
					target[key] = value["template_default"];
				}
				else if (target[key].type_name() != value["template_min"].type_name())
				{
					if (errors) errors->push_back("[mistyped: " + key + ", should be " + std::string(value["template_min"].type_name()) + ", but is " + std::string(target[key].type_name()) + "]");
					target[key] = value["template_default"];
				}
				else if (target[key] < value["template_min"] || target[key] > value["template_max"])
				{
					if (errors) errors->push_back("[misranged: " + key + "]");
					target[key] = Math::clamp(target[key], value["template_min"], value["template_max"]);
				}
			}
			else if (value.contains("template_min_length"))
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
				else if (target[key].get<std::string>().size() < value["template_min_length"].get<size_t>())
				{
					if (errors) errors->push_back("[string too short: " + key + "]");
				}
			}
			else if (value.is_object() && value.contains("template_multi_choice") && value["template_multi_choice"].is_array())
			{
				if (!target.contains(key))
				{
					if (errors) errors->push_back("[missing multi_choice: " + key + "]");
					target[key] = value["template_multi_choice"][0];
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
				add_missing_array(target[key], value, errors);
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

	void add_tag(Json& tags_array, const std::string& tag)
	{
		assert(tags_array.is_array());
		assert(tag.size() >= 3);
		auto it = std::find(tags_array.begin(), tags_array.end(), tag);
		if (it != tags_array.end())
			return;

		tags_array.push_back(tag);

		const auto dependencies = Parser::read_json_file(tag_dependencies_file);
		assert(dependencies.is_object());
		assert(dependencies.contains(tag));
		if (!dependencies[tag].contains("tags"))
			return;

		for (const auto& additional_tag : dependencies[tag]["tags"])
		{
			assert(additional_tag.is_string());
			assert(dependencies[tag]["tags"].is_array());
			add_tag(tags_array, additional_tag.get<std::string>());
		}
	}

	/* template_json will be a json object with key = component name, value = type, limits, delta, default.
	 * It will enforce the entity components. Requirements for entity components are they have to exist,
	 * they have to be the right type, and they have to respect number limits. It also has a default value,
	 * and a delta for Menu::Element.
	 * */
	Json get_template_json(const Json& entity, const bool get_all_definitions = false)
	{
		const auto definitions = Parser::read_json_file(component_definitions_file);
		if (get_all_definitions)
			return definitions;

		// filter by entity tags
		const auto dependencies = Parser::read_json_file(tag_dependencies_file);

		Json template_json = Json::object();
		template_json["tags"] = Json::array();

		// Add tags from dependencies, for example 'weapon' tag will have additionally 'item' tag
		// keys are all existing tags, values are objects with array of components and array of tags
		for (const auto& [key, value] : dependencies.items())
		{
			assert(entity["tags"].is_array());
			assert(template_json["tags"].is_array());
			assert(value.is_object());
			auto it = std::find(entity["tags"].begin(), entity["tags"].end(), key);
			if (it == entity["tags"].end())
				continue; // entity doesnt have this tag, don't add to template

			// add tag if its missing, and all its depndent tags recursively
			add_tag(template_json["tags"], key);
		}

		// template_json is now an object with only "tags" array
		// next add all necessary components to template_json

		const auto tags = template_json["tags"];
		for (const auto& tag_json : tags)
		{
			assert(tag_json.is_string());
			const auto tag = tag_json.get<std::string>();
			if (!dependencies[tag].contains("components"))
				continue;
			const auto components = create_from_keys(definitions, dependencies[tag]["components"]);
			template_json.update(components);
		}

		template_json["tags"] = tags;
		return template_json;
	}

	/* Tag dependencies declares all valid tags as keys inside a large object.
	 * */
	bool is_valid_tag(const std::string& tag)
	{
		const auto tags = Parser::read_json_file(tag_dependencies_file);
		for (const auto& [valid_tag, data] : tags.items())
		{
			if (tag == valid_tag)
				return true;
		}
		return false;
	}

	/* Iterate tags and remove invalid tags. For example "projectile" tag is removed, it is still existing
	 * in entities that added it previously. This function will find it and erase it and create an error message.
	 * */
	void fix_tags(Json& entity, std::vector<std::string>* errors = nullptr)
	{
		if (!entity.contains("tags"))
			Error::fatal("Entity has no tags");

		auto& tags = entity["tags"];
		for (auto it = tags.begin(); it != tags.end(); )
		{
			const auto tag = it->get<std::string>();
			if (!is_valid_tag(tag))
			{
				if (errors) errors->push_back("[invalid tag: " + tag + "]");
				it = tags.erase(it);
			}
			else
				it++;
		}
	}

	/* Aka auto-correct. Give default values.
	 * */
	void update_entity(Json& entity)
	{
		const auto template_json = get_template_json(entity);

		// Check if tags are real tags
		fix_tags(entity);

		// If entity has a tag that depends on some component, add them
		add_missing(entity, template_json);

		// Same but erase if tag was removed
		erase_redundant(entity, template_json);
	}

	/* Check if add_missing/auto-correct would change something if it had the chance.
	 * Give errors vector to know what it would change.
	 * */
	bool verify_entity(const Json& entity, std::vector<std::string>* errors = nullptr)
	{
		const auto template_json = get_template_json(entity);
		Json copy = entity;
		std::vector<std::string> dummy;
		if (errors == nullptr)
			errors = &dummy;
		fix_tags(copy, errors);
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
		const Json entities = Parser::read_json_file(entities_file);
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
		const Json entities = Parser::read_json_file(entities_file);

		static std::string search_word = "";
		static bool show_valid = true;
		static bool show_invalid = true;
		static Selection selection;
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
			Editor::print_json(subset);
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

		Json data = Parser::read_json_file(entities_file);
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
		std::ofstream file(entities_file);
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
			Editor::print_json(entity);

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
				add_tag(tags, tag);
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
		Selection selection;
		while (true)
		{
			Editor::print_json(entity);

			Menu menu(Screen::topright());
			menu.set_blocking(false);

			// Show entity name and maybe other stuff too
			const std::string name = entity["name"].get<std::string>();
			const Color color = Parser::parse_color(entity["color"]);
			const std::string colored_name = color.markup() + name + "{reset} ";

			std::vector<std::string> errors;
			const std::string valid_char = (verify_entity(entity, &errors) ? "🟢" : "🔴");

			const std::string head = valid_char + " " + colored_name;

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
				//Log::log("entity: " + key + ": " + value.dump(4));
				//Log::log("template": " + key + ": " + template_json[key].dump(4));
				Type type = Type::None;

				if (value.is_number())
				{
					type = Type::ValueSelector;
					const Range<int> range(
							template_json[key]["template_min"].get<int>(),
							template_json[key]["template_max"].get<int>()
							);
					const double delta = template_json[key]["template_delta"].get<double>();
					menu.add_element(Element(
								Type::ValueSelector,
								key,
								&value,
								range,
								delta
								));
					continue;
				}
				else if (value.contains("range") && template_json[key].contains("template_range") &&
						value["range"].is_array() && template_json[key]["template_range"].is_array() &&
						value["range"].size() == 2 && template_json[key]["template_range"].size() == 2)
				{
					const Range<int> range(
							template_json[key]["template_range"][0].get<int>(),
							template_json[key]["template_range"][1].get<int>()
							);
					const double delta = template_json[key]["template_delta"].get<double>();
					Element e(
							Type::RangeSelector,
							key,
							&value["range"], // give the array as value to Element
							range,
							delta
							);
					menu.add_element(e);
					continue;
				}
				else if (value.is_string() && template_json[key].is_object() && template_json[key].contains("template_multi_choice"))
				{
					Element e(
							Type::MultiChoice,
							key,
							&value,
							template_json[key]["template_multi_choice"].get<std::vector<std::string>>()
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
				if (entity == part && part != load_entity(entity["name"]))
				{
					if (Dialog::confirm("There are unsaved changes. Continue editing?"))
						continue;
				}
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
			selection = Dialog::get_selection("* Entity Editor *", buttons, Screen::middle(), selection.index);
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

};
