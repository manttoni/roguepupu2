#include <codecvt>
#include <locale>
#include <filesystem>                                     // for path
#include <fstream>                                        // for basic_ifstream
#include <nlohmann/json.hpp>                              // for basic_json
#include <regex>                                          // for regex_match
#include <string>                                         // for string, ope...
#include "Color.hpp"                                      // for Color
#include "Components.hpp"                                 // for Resources
#include "ECS.hpp"                                        // for get_fatigue...
#include "EntityFactory.hpp"                              // for EntityFactory
#include "Utils.hpp"                                      // for error, rand...
#include "entt.hpp"                                       // for operator==
#include "systems/EquipmentSystem.hpp"                    // for equip
#include "systems/ActionSystem.hpp"
#include "Event.hpp"
#include "AbilityDatabase.hpp"
#include "Parser.hpp"
class Cell;  // lines 18-18

#define CELL_SIZE 5
#define MELEE_RANGE 1.5
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
	Log::log("Parsing " + path.string());
	std::ifstream file(path);
	if (!file.is_open())
		Log::error(std::string("Opening file failed: ") + path.string());
	if (file.peek() == std::ifstream::traits_type::eof())
	{
		Log::log("Empty file: " + path.string());
		file.close();
		return;
	}
	nlohmann::json definitions;
	file >> definitions;
	file.close();
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
		{
			(void) data;
			reg.template emplace<Solid>(e);
		}
	},
	{ "opaque", [](auto& reg, auto e, const nlohmann::json& data)
		{
			(void) data;
			reg.template emplace<Opaque>(e);
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
			for (const auto& entity : data)
			{
				const size_t amount = entity.contains("amount") ? entity["amount"].get<size_t>() : 1;
				if (!entity.contains("filter")) // Need to specify what kind of entities it has with a filter
					Log::error("Cannot spawn entity to inventory: " + entity.dump(4));
				if (entity.contains("chance") && Random::randreal(0, 1) > entity["chance"].get<double>())
					continue;
				std::vector<std::string> names = EntityFactory::instance().random_pool(entity["filter"], amount);
				for (size_t i = 0; i < amount; ++i)
				{
					const size_t rand = Random::randsize_t(0, names.size() - 1);
					inventory.push_back(EntityFactory::instance().create_entity(reg, names[rand]));
				}
			}
			reg.template emplace<Inventory>(e, inventory);
		}
	},
	{ "vision", [](auto& reg, auto e, const nlohmann::json& data)
		{	// Can see and has a vision range
			reg.template emplace<Vision>(e, data.get<int>());
		}
	},
	{ "glow", [](auto& reg, auto e, const nlohmann::json& data)
		{	// Emits light around it
			reg.template emplace<Glow>(e, data["strength"].get<double>(), data["radius"].get<double>());
		}
	},
	{ "player", [](auto& reg, auto e, const nlohmann::json& data)
		{	// Is the player, one and only
			(void) data;
			reg.template emplace<Player>(e);
		}
	},
	{ "weight", [](auto& reg, auto e, const nlohmann::json& data)
		{
			reg.template emplace<Weight>(e, data.get<double>());
		}
	},
	{ "equipment", [](auto& reg, auto e, const nlohmann::json& data)
		{	// This entity is a piece of equipment that can be equipped in slots
			reg.template emplace<Equipment>(e, Equipment::from_string(data.get<std::string>()));
		}
	},
	{ "equipment_slots", [](auto& reg, auto e, const nlohmann::json& data)
		{
			reg.template emplace<EquipmentSlots>(e);
			(void) data;
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
		{
			const std::string faction = data.get<std::string>();
			reg.template emplace<Faction>(e, faction);
		}
	},
	{ "glyph", [](auto& reg, auto e, const nlohmann::json& data)
		{
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
			Color color = Color(data[0].get<int>(), data[1].get<int>(), data[2].get<int>());
			reg.template emplace<FGColor>(e, color);
		}
	},
	{ "bgcolor", [](auto& reg, auto e, const nlohmann::json& data)
		{
			const auto& c = data["color"];
			Color color = Color(c[0].get<int>(), c[1].get<int>(), c[2].get<int>());
			reg.template emplace<BGColor>(e, color);
		}
	},
	{ "attributes", [](auto& reg, auto e, const nlohmann::json& data)
		{
			const int strength = data["strength"].get<int>();
			const int dexterity = data["dexterity"].get<int>();
			const int intelligence = data["intelligence"].get<int>();
			reg.template emplace<Attributes>(e, strength, dexterity, intelligence);
		}
	},
	{ "two_handed", [](auto& reg, auto e, const nlohmann::json& data)
		{
			(void) data;
			reg.template emplace<TwoHanded>(e);
		}
	},
	{ "level", [](auto& reg, auto e, const nlohmann::json& data)
		{
			reg.template emplace<Level>(e, data.get<int>());
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
	{ "landmark", [](auto& reg, auto e, const nlohmann::json& data)
		{	// this entity will be remembered by player even if outside of vision
			reg.template emplace<Landmark>(e);
			(void) data;
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
				if (entry.contains("target"))
					trigger.target = Parser::parse_target(entry["target"]);
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

entt::entity EntityFactory::create_entity(entt::registry& registry, const std::string& name, Cell* cell)
{
	if (LUT.find(name) == LUT.end())
		Log::error("Entity not found: " + name);

	auto entity = registry.create();
	const auto& data = LUT[name];
	for (const auto& [field_name, field_data] : data.items())
	{
		auto it = field_parsers.find(field_name);
		if (it == field_parsers.end())
			Log::error("Unknown field name: " + field_name);
		it->second(registry, entity, field_data);
	}

	if (cell != nullptr)
		registry.emplace<Position>(entity, cell);

	// always recalculate max hp
	if (registry.all_of<Attributes>(entity))
	{
		registry.emplace<Resources>(entity,
				ECS::get_health_max(registry, entity),
				ECS::get_fatigue_max(registry, entity),
				ECS::get_mana_max(registry, entity));
	}

	// Equip items
	if (registry.all_of<Inventory>(entity))
	{
		for (const auto item : registry.get<Inventory>(entity).inventory)
		{
			if (registry.all_of<Equipment>(item))
				EquipmentSystem::equip(registry, entity, item);
		}
	}
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
