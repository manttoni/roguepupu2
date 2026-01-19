#include <codecvt>
#include <locale>
#include <filesystem>                                     // for path
#include <fstream>                                        // for basic_ifstream
#include <nlohmann/json.hpp>                              // for basic_json
#include <regex>                                          // for regex_match
#include <string>                                         // for string, ope...
#include "systems/state/StateSystem.hpp"
#include "components/Components.hpp"                                 // for Resources
#include "database/AbilityDatabase.hpp"
#include "database/EntityFactory.hpp"                              // for EntityFactory
#include "domain/Color.hpp"                                      // for Color
#include "utils/Parser.hpp"
#include "utils/Random.hpp"
#include "external/entt/entt.hpp"
#include "utils/ECS.hpp"
#include "domain/Event.hpp"

void EntityFactory::init()
{
	const std::filesystem::path root = "data/entities";
	for (const auto& entry : std::filesystem::recursive_directory_iterator(root))
	{
		if (!entry.is_regular_file())
			continue;
		if (entry.path().extension() != ".json")
			continue;
		read_definitions(entry.path());
	}
	Log::log("Entities parsed");
}

void EntityFactory::read_definitions(const std::filesystem::path& path)
{
	nlohmann::json definitions = Parser::read_file(path);
	const std::string category = path.parent_path().filename();
	const std::string subcategory = path.stem().filename();
	add_entities(definitions, category, subcategory);
}

using FieldParser = std::function<void(entt::registry&, entt::entity, const nlohmann::json&)>;

std::unordered_map<std::string, FieldParser> field_parsers =
{
	{ "name", [](auto& reg, auto e, const nlohmann::json& data)
		{
			reg.template emplace<Name>(e, data.get<std::string>());
		}
	},
	{ "solid", [](auto& reg, auto e, const nlohmann::json& data)
		{	// Can't move through solid entities
			if (!data.is_boolean())
				Log::error("Solid should be boolean: " + data.dump(4));
			if (data.get<bool>() == true)
				reg.template emplace<Solid>(e);
		}
	},
	{ "opaque", [](auto& reg, auto e, const nlohmann::json& data)
		{
			if (!data.is_number() || data.get<double>() < 0 || data.get<double>() > 1)
				Log::error("Opaqueness should be number [0,1]");
			reg.template emplace<Opaque>(e, data.get<double>());
		}
	},
	{ "inventory", [](auto& reg, auto e, const nlohmann::json& data)
		{	// Can store entities inside it, and might do so initially
			std::vector<entt::entity> inventory;
			if (data.is_boolean())
			{
				if (data.get<bool>() == true)
					reg.template emplace<Inventory>(e, inventory);
				return;
			}
			if (!data.is_array())
				Log::error("Inventory format error: " + data.dump(4));

			for (const auto& entry : data)
			{
				size_t amount = 1;
				if (entry.contains("amount"))
				{
					const auto [min, max] = Parser::parse_range(entry["amount"]);
					amount = Random::randsize_t(min, max);
				}
				std::vector<std::string> name_pool;
				if (entry.contains("id"))
					name_pool.push_back(entry["id"].get<std::string>());
				else if (entry.contains("filter"))
					name_pool = EntityFactory::instance()
						.random_pool(entry["filter"], amount);
				if (name_pool.empty())
					Log::error("Missing id or invalid filter: " + data.dump(4));
				for (size_t i = 0; i < amount; ++i)
				{
					const size_t random_index = Random::randsize_t(0, name_pool.size() - 1);
					assert(random_index < name_pool.size());
					const entt::entity e = EntityFactory::instance()
						.create_entity(reg, name_pool[random_index]);
					inventory.push_back(e);
				}
			}
			reg.template emplace<Inventory>(e, inventory);
		}
	},
	{ "vision", [](auto& reg, auto e, const nlohmann::json& data)
		{	// Can see and has a vision range. This might get replaced by some perception stat, probably will
			if (!data.is_number() || data.get<double>() < 0)
				Log::error("Vision range should be positive number");
			reg.template emplace<Vision>(e, data.get<double>());
		}
	},
	{ "glow", [](auto& reg, auto e, const nlohmann::json& data)
		{	// Emits light around it
			if (!data.contains("intensity") || !data["intensity"].is_number() ||
				!data.contains("radius") || !data["radius"].is_number())
				Log::error("Glow requires intensity and radius numbers: " + data.dump(4));
			reg.template emplace<Glow>(e, data["intensity"].get<double>(), data["radius"].get<double>());
		}
	},
	{ "player", [](auto& reg, auto e, const nlohmann::json& data)
		{	// Is a playable character
			if (!data.is_boolean())
				Log::error("Player should be boolean: " + data.dump(4));
			if (data.get<bool>() == true)
				reg.template emplace<Player>(e);
		}
	},
	{ "weight", [](auto& reg, auto e, const nlohmann::json& data)
		{
			if (!data.is_number() || data.get<double>() < 0)
				Log::error("Weight should be positive number: " + data.dump(4));
			reg.template emplace<Weight>(e, data.get<double>());
		}
	},
	{ "equipment", [](auto& reg, auto e, const nlohmann::json& data)
		{	// This entity is a piece of equipment that can be equipped in slots
			if (!data.is_string())
				Log::error("Equipment (slot) should be string");
			reg.template emplace<Equipment>(e, Equipment::from_string(data.get<std::string>()));
		}
	},
	{ "equipment_slots", [](auto& reg, auto e, const nlohmann::json& data)
		{
			if (!data.is_boolean())
				Log::error("EquipmentSlots should be boolean");
			if (data.get<bool>() == true)
				reg.template emplace<EquipmentSlots>(e);
		}
	},
	{ "category", [](auto& reg, auto e, const nlohmann::json& data)
		{	// Main category like "weapons" or "potions". Comes from directory name
			reg.template emplace<Category>(e, data.get<std::string>());
		}
	},
	{ "subcategory", [](auto& reg, auto e, const nlohmann::json& data)
		{
			reg.template emplace<Subcategory>(e, data.get<std::string>());
		}
	},
	{ "faction", [](auto& reg, auto e, const nlohmann::json& data)
		{	// this will get updated at some point when plans are ready
			const std::string faction = data.get<std::string>();
			reg.template emplace<Faction>(e, faction);
		}
	},
	{ "glyph", [](auto& reg, auto e, const nlohmann::json& data)
		{
			if (!data.is_string())
				Log::error("Glyph should be string: " + data.dump(4));
			const std::string glyph_str = data.get<std::string>();
			std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
			std::wstring wstr = conv.from_bytes(glyph_str);
			if (wstr.size() != 1)
				Log::error("Glyph is too wide");
			const wchar_t glyph = wstr[0];
			reg.template emplace<Glyph>(e, glyph);
		}
	},
	{ "fgcolor", [](auto& reg, auto e, const nlohmann::json& data)
		{
			if (!data.is_array() || data.size() != 3)
				Log::error("Invalid color values: " + data.dump(4));
			Color color = Color(data[0].get<int>(), data[1].get<int>(), data[2].get<int>());
			reg.template emplace<FGColor>(e, color);
		}
	},
	{ "bgcolor", [](auto& reg, auto e, const nlohmann::json& data)
		{
			if (!data.is_array() || data.size() != 3)
				Log::error("Invalid color values: " + data.dump(4));
			const auto& c = data["color"];
			Color color = Color(c[0].get<int>(), c[1].get<int>(), c[2].get<int>());
			reg.template emplace<BGColor>(e, color);
		}
	},
	{ "level", [](auto& reg, auto e, const nlohmann::json& data)
		{
			if (!data.is_number() || data.get<int>() < 0)
				Log::error("Level should be positive integer: " + data.dump(4));

			const size_t xp = StateSystem::level_to_xp(data.get<size_t>());
			reg.template emplace<Experience>(e, xp);
		}
	},
	{ "abilities", [](auto& reg, auto e, const nlohmann::json& data)
		{
			if (!data.is_array())
				Log::error("Abilities not an array: " + data.dump(4));
			std::map<std::string, Ability> abilities;
			for (const auto& ability : data)
			{
				const auto id = ability.get<std::string>();
				const auto& database = reg.ctx().template get<AbilityDatabase>();
				abilities[id] = database.get_ability(id);
			}
			reg.template emplace<Abilities>(e, abilities);
		}
	},
	{ "ai", [](auto& reg, auto e, const nlohmann::json& data)
		{
			if (!data.is_array())
				Log::error("AI not an array: " + data.dump(4));
			std::vector<Intent> intentions;
			for (const auto& entry : data)
			{
				Intent intent;
				if (entry.contains("type"))
				{
					const auto& type = entry["type"].get<std::string>();
					if (type == "hide")
						intent.type = Intent::Type::Hide;
					else if (type == "attack")
						intent.type = Intent::Type::Attack;
					else if (type == "flee")
						intent.type = Intent::Type::Flee;
					else if (type == "use_ability")
						intent.type = Intent::Type::UseAbility;
					else if (type == "gather")
						intent.type = Intent::Type::Gather;
					else
						Log::error("Unknown Intent type: " + type);
				}	else Log::error("No intent type: " + entry.dump(4));
				if (entry.contains("ability_id"))
					intent.ability_id = entry["ability_id"].get<std::string>();
				if (entry.contains("conditions"))
					intent.conditions = Parser::parse_conditions(entry["conditions"]);
				intentions.push_back(intent);
			}
			reg.template emplace<AI>(e, intentions);
		}
	},
	{ "spawn", [](auto& reg, auto e, const nlohmann::json& data)
		{	// unused at this point
			(void) reg; (void) e; (void) data;
		}
	},
	{ "triggers", [](auto& reg, auto e, const nlohmann::json& data)
		{
			if (!data.is_array())
				Log::error("Triggers not an array: " + data.dump(4));

			std::vector<Trigger> triggers;
			for (const auto& entry : data)
			{
				if (!entry.is_object())
					Log::error("Trigger not object: " + entry.dump(4));
				Trigger trigger;
				if (entry.contains("type"))
				{
					const auto& type = entry["type"].get<std::string>();
					if (type == "enter_cell")
						trigger.type = Trigger::Type::EnterCell;
					else if (type == "gather")
						trigger.type = Trigger::Type::Gather;
					else Log::error("Unkown trigger type: " + type);
				} else Log::error("Trigger has no type: " + entry.dump(4));
				if (entry.contains("effect"))
					trigger.effect = Parser::parse_effect(entry["effect"]);
				if (entry.contains("conditions"))
					trigger.conditions = Parser::parse_conditions(entry["conditions"]);
				triggers.push_back(trigger);
			}
			reg.template emplace<Triggers>(e, triggers);
		}
	},
	{ "gatherable", [](auto& reg, auto e, const nlohmann::json& data)
		{
			const auto tool = data.contains("tool") ?
				Tool::from_string(data["tool"].get<std::string>()) :
				Tool::Type::None;
			const auto& entity = data.contains("entity_id") ?
				data["entity_id"].get<std::string>() :
				"";
			const auto amount = data.contains("amount") ?
				data["amount"].get<size_t>() :
				1;
			reg.template emplace<Gatherable>(e, tool, entity, amount);
		}
	},
	{ "tool", [](auto& reg, auto e, const nlohmann::json& data)
		{
			reg.template emplace<Tool>(e, Tool::from_string(data.get<std::string>()));
		}
	},
	{ "transition", [](auto& reg, auto e, const nlohmann::json& data)
		{
			if (data.is_boolean() && data == true)
				reg.template emplace<Transition>(e, entt::null);
		}
	},
	{ "edge", [](auto& reg, auto e, const nlohmann::json& data)
		{
			if (!data.is_number_float())
				Log::error("Edge is not a float");
			const double edge = data.get<double>();
			if (edge < 0 || edge > 1)
				Log::error("Invalid Edge value");
			reg.template emplace<Edge>(e, edge);
		}
	},
	{ "throwable", [](auto& reg, auto e, const nlohmann::json& data)
		{
			if (!data.is_boolean())
				Log::error("Throwable value type error");
			if (data.get<bool>() == true)
				reg.template emplace<Throwable>(e);
		}
	},
	{ "size", [](auto& reg, auto e, const nlohmann::json& data)
		{
			double size = 0;
			if (data.is_string() && data.get<std::string>() == "-inf")
				size = -std::numeric_limits<double>::infinity();
			else if (data.is_string() && data.get<std::string>() == "inf")
				size = std::numeric_limits<double>::infinity();
			else if (data.is_number())
				size = data.get<double>();
			else Log::error("Unhandled size type: " + data.dump(4));
			reg.template emplace<Size>(e, size);
		}
	}
};

// Add json data into map, and add categories to all of them
void EntityFactory::add_entities(nlohmann::json& json, const std::string& category, const std::string& subcategory)
{
	for (const auto& [name, data] : json.items())
	{
		if (!data.contains("category"))
			data["category"] = category;
		if (!data.contains("subcategory"))
			data["subcategory"] = subcategory;
		if (!data.contains("name"))
			data["name"] = name;
		LUT[name] = data;
	}
}

entt::entity EntityFactory::create_entity(entt::registry& registry, const std::string& name, const std::optional<Position>& position)
{
	Log::log("Creating entity: " + name);
	if (LUT.find(name) == LUT.end())
		Log::error("Entity not found: " + name);

	auto entity = registry.create();
	const auto& data = LUT[name];
	for (const auto& [field_name, field_data] : data.items())
	{
		auto it = field_parsers.find(field_name);
		if (it == field_parsers.end())
			Log::error("Unknown field name: " + field_name);
		Log::log("Field name: " + field_name);
		it->second(registry, entity, field_data);
	}

	if (!registry.all_of<Category, Subcategory, Name>(entity))
		Log::error("Entity doesn't have core components");

	if (position.has_value())
	{
		registry.emplace<Position>(entity, *position);
		Event spawn_event;
		spawn_event.effect.type = Effect::Type::Spawn;
		spawn_event.target.entity = entity;
		spawn_event.target.position = *position;
		ECS::queue_event(registry, spawn_event);
	}

	Log::log("Entity created");
	return entity;
}

bool EntityFactory::exclude(const nlohmann::json& data, const nlohmann::json& filter)
{
	for (const auto& [filter_field, filter_data] : filter.items())
	{
		if (filter_data == "any")
		{
			if (data.contains(filter_field))
				continue;
			else
				return true;
		}
		if (filter_data == "none")
		{
			if (data.contains(filter_field))
				return true;
			else
				continue;
		}
		if (!data.contains(filter_field) || data[filter_field] != filter_data)
			return true;
	}
	return false;
}

std::vector<std::string> EntityFactory::random_pool(const nlohmann::json& filter, const size_t amount)
{
	std::vector<std::string> pool;
	for (const auto& [name, data] : LUT)
	{
		if (!exclude(data, filter))
			pool.push_back(name);
	}
	std::shuffle(pool.begin(), pool.end(), Random::rng());
	if (amount < pool.size())
		pool.resize(amount);
	return pool;
}

std::vector<std::string> EntityFactory::get_category_names() const
{
	std::vector<std::string> category_names;
	for (const auto& [name, data] : LUT)
	{
		if (!data.contains("category"))
			Log::error("Uncategorized entity: " + name);
		const auto category = data["category"].get<std::string>();
		auto it = std::find(category_names.begin(), category_names.end(), category);
		if (it == category_names.end())
			category_names.push_back(category);
	}
	return category_names;
}

std::vector<std::string> EntityFactory::get_subcategory_names(const std::string& category) const
{
	std::vector<std::string> subcategory_names;
	for (const auto& [name, data] : LUT)
	{
		if (!data.contains("category") || !data.contains("subcategory"))
			Log::error("Uncategorized entity: " + name);
		if (data["category"].get<std::string>() != category)
			continue;
		const auto subcategory = data["subcategory"].get<std::string>();
		auto it = std::find(subcategory_names.begin(), subcategory_names.end(), subcategory);
		if (it == subcategory_names.end())
			subcategory_names.push_back(subcategory);
	}
	return subcategory_names;
}
