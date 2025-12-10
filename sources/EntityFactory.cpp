#include <filesystem>                                     // for path
#include <fstream>                                        // for basic_ifstream
#include <nlohmann/detail/iterators/iter_impl.hpp>        // for iter_impl
#include <nlohmann/detail/iterators/iteration_proxy.hpp>  // for iteration_p...
#include <nlohmann/json.hpp>                              // for basic_json
#include <nlohmann/json_fwd.hpp>                          // for json
#include <string>                                         // for string, ope...
#include "Color.hpp"                                      // for Color
#include "Components.hpp"                                 // for Position
#include "Dice.hpp"                                       // for Dice
#include "EntityFactory.hpp"                              // for EntityFactory
#include "Utils.hpp"                                      // for error, rand...
#include "entt.hpp"                                       // for vector, ope...
#include "systems/DamageSystem.hpp"                       // for parse_type
class Cell;

#define CELL_SIZE 5
#define MELEE_RANGE 1.5
void EntityFactory::init()
{
	read_definitions("data/items/weapons.json");
	read_definitions("data/items/armor.json");
	read_definitions("data/plants/mushrooms.json");
	read_definitions("data/furniture/chests.json");
	read_definitions("data/creatures/players.json");
	read_definitions("data/creatures/goblins.json");
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
	{ "renderable", [](auto& reg, auto e, const nlohmann::json& data)
		{	// If the values are not defined, will define them later
			wchar_t glyph = L'?';
			Color color;
			if (data.contains("glyph"))
			{
				const std::string glyph_str = data["glyph"].get<std::string>();
				glyph = static_cast<wchar_t>(glyph_str[0]);
			}
			if (data.contains("color"))
			{
				const auto& c = data["color"];
				color = Color(c[0].get<int>(), c[1].get<int>(), c[2].get<int>());
			}
			reg.template emplace<Renderable>(e, glyph, color);
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
			for (const auto& entity : data)
			{
				const std::string amount_str = entity.contains("amount") ? entity["amount"].get<std::string>() : "1d1";
				const Dice d(amount_str);
				const size_t amount = d.roll();

				if (!entity.contains("filter")) // Need to specify what kind of entities it has with a filter
					Log::error("Cannot spawn entity to inventory: " + entity.dump(4));
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
	{ "damage", [](auto& reg, auto e, const nlohmann::json& data)
		{	// Can deal damage
			if (!data.contains("type") || !data.contains("dice"))
				Log::error("Damage component incomplete: " + data.dump(4));

			std::string type = data["type"].get<std::string>();
			Dice dice(data["dice"].get<std::string>());
			reg.template emplace<Damage>(e, type, dice);
		}
	},
	{ "weapon", [](auto& reg, auto e, const nlohmann::json& data)
		{
			if (!data.contains("proficiency"))
				Log::error("Weapon component missing proficiency: " + data.dump(4));

			const std::string proficiency = data["proficiency"].get<std::string>();
			std::vector<std::string> properties = data["properties"].get<std::vector<std::string>>();

			// Parse more data from properties
			double normal_range = MELEE_RANGE, long_range = MELEE_RANGE;
			Dice versatile_dice;
			for (auto& property : properties)
			{
				const size_t space = property.find(' '); // splits additional data
				const size_t slash = property.find('/'); // normal/long range
				const size_t parenth = property.find('('); // (versatile dice)
				if (slash != std::string::npos)
				{
					normal_range = std::stod(property.substr(space + 1)) / CELL_SIZE;
					long_range = std::stod(property.substr(slash + 1)) / CELL_SIZE;
				}
				if (parenth != std::string::npos)
					versatile_dice = Dice(property.substr(parenth + 1));
				if (space != std::string::npos)
					property = property.substr(0, space);
			}
			reg.template emplace<Weapon>(e, proficiency, properties, normal_range, long_range, versatile_dice);
		}
	},
	{ "value", [](auto& reg, auto e, const nlohmann::json& data)
		{
			const size_t value = data.get<size_t>();
			reg.template emplace<Value>(e, value);
		}
	},
	{ "weight", [](auto& reg, auto e, const nlohmann::json& data)
		{	// Has a weight in lb
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
	{ "rarity", [](auto& reg, auto e, const nlohmann::json& data)
		{
			reg.template emplace<Rarity>(e, data.get<std::string>());
		}
	},
	{ "armor", [](auto& reg, auto e, const nlohmann::json& data)
		{
			if (!data.contains("proficiency"))
				Log::error("Incomplete armor component: " + data.dump(4));

			const std::string proficiency = data["proficiency"].get<std::string>();
			const size_t armor_class = data.contains("armor_class") ? data["armor_class"].get<size_t>() : 0;
			const size_t max_dexbonus = data.contains("max_dexbonus") ? data["max_dexbonus"].get<size_t>() : SIZE_MAX;
			const std::string stealth = data.contains("stealth") ? data["stealth"].get<std::string>() : "";
			const size_t strength_requirement = data.contains("strength_requirement") ? data["strength_requirement"].get<size_t>() : 0;
			reg.template emplace<Armor>(e, proficiency, armor_class, max_dexbonus, stealth, strength_requirement);
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
	return entity;
}

bool EntityFactory::exclude(const nlohmann::json& data, const nlohmann::json& filter)
{
	for (const auto& [filter_field, filter_data] : filter.items())
	{
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
