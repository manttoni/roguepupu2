#include <nlohmann/json.hpp>
#include <fstream>

#include "external/entt/entt.hpp"
#include "domain/World.hpp"
#include "generation/CaveGenerator.hpp"
#include "utils/ECS.hpp"
#include "utils/Parser.hpp"
#include "UI/Dialog.hpp"
#include "core/paths.hpp"
#include "editor/Editor.hpp"
#include "editor/CaveEditor.hpp"
#include "UI/Menu.hpp"

namespace CaveEditor
{
	using Element = Menu::Element;
	using Type = Element::Type;
	using Selection = Menu::Selection;
	using Json = nlohmann::json;

	Json new_config()
	{
		Json config = Json::object();
		config["density"]["frequency"] = 0.01;
		config["density"]["octaves"] = 1;
		config["erosion"]["main"] = 0.01;
		config["erosion"]["secondary"] = 0.01;
		config["smooth"]["intensity"] = 0.01;
		config["smooth"]["iterations"] = 1;
		config["smooth"]["rock"] = false;
		config["features"]["sinks"] = 1;
		config["features"]["sources"] = 1;
		config["margin"]["size"] = 1;
		return config;
	}

	Json load_config()
	{
		const auto configs = Parser::read_json_file(cave_configs_file);
		std::vector<std::string> buttons;
		for (const auto& [id, data] : configs.items())
			buttons.push_back(id);
		Selection selection = Dialog::get_selection("Load Config", buttons, Screen::middle(), selection.index);
		if (selection.cancelled)
			return Json{};
		return configs[selection.element->label];
	}

	void save_config(const Json& config)
	{
		auto configs = Parser::read_json_file(cave_configs_file);
		std::string id = "";
		const Selection selection = Dialog::get_input("Save as:", &id);
		if (selection.cancelled)
			return;
		configs[id] = config;
		std::ofstream file(cave_configs_file);
		file << config.dump(4);
		Dialog::alert("Config saved");
	}

	void generate_cave(const Json& config)
	{
		entt::registry registry;
		ECS::init_registry(registry);
		const size_t cave_idx = registry.ctx().get<World>().new_cave();
		CaveGenerator::Data data(registry, ECS::get_cave(registry, cave_idx));
		Parser::parse_cave_generation_conf(config, data);
		CaveGenerator::generate_cave(data, false);
	}

	void edit_config(Json& config)
	{
		bool auto_generate = true;

		Selection selection;
		while (true)
		{
			if (auto_generate)
				generate_cave(config);
			Menu menu(Screen::topright());
			menu.set_blocking(false);

			menu.add_element(Element::text("Edit Config"));
			menu.add_element(Element::line());

			menu.add_element(Element(Type::Checkbox, "Auto generate", &auto_generate));

			for (const auto& [key, value] : config.items())
			{
				for (const auto& [inner_key, inner_value] : value.items())
				{
					if (inner_value.is_number())
					{
						const double delta = inner_value.is_number_float() ?
							0.01 : 1;
						menu.add_element(Element(
									Type::ValueSelector,
									inner_key,
									&inner_value,
									Range<int>(0, 16),
									delta
									));
					}
					else if (inner_value.is_boolean())
					{
						menu.add_element(Element(
									Type::Checkbox,
									inner_key,
									&inner_value
									));
					}
					else
						Error::fatal("Unhandled cave config setting type");
				}
			}

			menu.add_element(Element::confirm("Save"));
			menu.add_element(Element::cancel());

			selection = menu.get_selection(selection.index);
			if (selection.cancelled)
				break;
			if (selection.confirmed)
				save_config(config);
			if (selection.timed_out)
				continue;
		}
	}

	void cave_editor()
	{
		static const std::vector<std::string> buttons = {
			"New Config", "Load Config", "Back"
		};
		Selection selection;
		while (true)
		{
			Json config;
			selection = Dialog::get_selection("* Cave Config Editor *", buttons, Screen::middle(), selection.index);
			if (selection.cancelled)
				break;

			assert(selection.element.has_value());
			const auto label = selection.element->label;
			if (label == "New Config")
				config = new_config();
			else if (label == "Load Config")
			{
				config = load_config();
				if (config == Json{})
					continue;
			}

			edit_config(config);
		}
	}
};
