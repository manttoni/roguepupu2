#include <regex>
#include <filesystem>                                     // for path
#include <fstream>                                        // for basic_ifstream
#include <nlohmann/detail/iterators/iter_impl.hpp>        // for iter_impl
#include <nlohmann/detail/iterators/iteration_proxy.hpp>  // for iteration_p...
#include <nlohmann/json.hpp>                              // for basic_json
#include <nlohmann/json_fwd.hpp>                          // for json
#include <string>                                         // for string, ope...
#include "Color.hpp"                                      // for Color
#include "Components.hpp"                                 // for Position
#include "EntityFactory.hpp"                              // for EntityFactory
#include "Utils.hpp"                                      // for error, rand...
#include "entt.hpp"                                       // for vector, ope...
#include "systems/DamageSystem.hpp"                       // for parse_type
#include "ECS.hpp"
#include "systems/StatsSystem.hpp"
#include "systems/EquipmentSystem.hpp"
class Cell;

#define CELL_SIZE 5
#define MELEE_RANGE 1.5
void EntityFactory::init()
{
	read_definitions("data/entities/items/weapons.json");
	read_definitions("data/entities/items/armor.json");
	read_definitions("data/entities/plants/mushrooms.json");
	read_definitions("data/entities/furniture/chests.json");
	read_definitions("data/entities/creatures/players.json");
	read_definitions("data/entities/creatures/goblins.json");
	Log::log("Parsed entities amount: " + std::to_string(LUT.size()));
	for (const auto& [name, data] : LUT)
	{
		Log::log(data.dump(4));
	}
}

void EntityFactory::read_definitions(const std::filesystem::path& path)
{
	std::ifstream file(path);
	if (!file.is_open())
		Log::error(std::string("Opening file failed: ") + path.string());
	nlohmann::json definitions;
	file >> definitions;
	file.close();
	const std::string category = path.parent_path().filename();
	const std::string subcategory = path.stem().filename();
	add_entities(definitions, category, subcategory);
	Log::log(path.string() + " parsed");
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
		{	// This entity has equipment slots and can equip items
			(void) data;
			reg.template emplace<Equipment>(e);
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
			const wchar_t glyph = static_cast<wchar_t>(glyph_str[0]);
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
	{ "armor_penetration", [](auto& reg, auto e, const nlohmann::json& data)
		{
			reg.template emplace<ArmorPenetration>(e, data.get<int>());
		}
	},
	{ "armor", [](auto& reg, auto e, const nlohmann::json& data)
		{
			reg.template emplace<Armor>(e, data.get<int>());
		}
	},
	{ "accuracy", [](auto& reg, auto e, const nlohmann::json& data)
		{
			reg.template emplace<Accuracy>(e, data.get<int>());
		}
	},
	{ "evasion", [](auto& reg, auto e, const nlohmann::json& data)
		{
			reg.template emplace<Evasion>(e, data.get<int>());
		}
	},
	{ "barrier", [](auto& reg, auto e, const nlohmann::json& data)
		{
			reg.template emplace<Barrier>(e, data.get<int>());
		}
	},
	{ "power", [](auto& reg, auto e, const nlohmann::json& data)
		{
			reg.template emplace<Power>(e, data.get<int>());
		}
	},
	{ "crit_chance", [](auto& reg, auto e, const nlohmann::json& data)
		{
			reg.template emplace<CritChance>(e, data.get<double>());
		}
	},
	{ "crit_multiplier", [](auto& reg, auto e, const nlohmann::json& data)
		{
			reg.template emplace<CritMultiplier>(e, data.get<double>());
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
	{ "damage", [](auto& reg, auto e, const nlohmann::json& data)
		{
			if (!data.contains("amount") || !data.contains("type"))
				Log::error("Error parsing entity: " + data.dump(4));

			const std::regex pattern(R"(^\d+-\d+$)");
			const std::string str = data["amount"].get<std::string>();
			if (!std::regex_match(str, pattern))
				Log::error("Invalid damage: " + str);
			const auto dash = str.find('-');
			const int min = std::stoi(str.substr(0, dash));
			const int max = std::stoi(str.substr(dash + 1));
			reg.template emplace<Damage>(e, min, max, data["type"].get<std::string>());
		}
	},
	{ "level", [](auto& reg, auto e, const nlohmann::json& data)
		{
			reg.template emplace<Level>(e, data.get<int>());
		}
	},
	{ "actions", [](auto& reg, auto e, const nlohmann::json& data)
		{
			reg.template emplace<Actions>(e, data.get<int>());
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
		if (!data.contains("actions"))
			data["actions"] = 1;
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
	if (registry.all_of<Attributes>(entity))
	{
		registry.emplace<Resources>(entity,
				ECS::get_health_max(registry, entity),
				ECS::get_fatigue_max(registry, entity),
				ECS::get_mana_max(registry, entity));
	}
	if (registry.all_of<Inventory>(entity))
	{
		for (const auto item : registry.get<Inventory>(entity).inventory)
		{
			if (ECS::is_equippable(registry, item))
				EquipmentSystem::equip(registry, entity, item);
		}
	}
	return entity;
}

bool EntityFactory::exclude(const nlohmann::json& data, const nlohmann::json& filter)
{
	for (const auto& [filter_field, filter_data] : filter.items())
	{
		if (data.contains("player"))
			return true; // always exclude players
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
