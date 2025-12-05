#include <fstream>
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <unordered_map>
#include <iostream>
#include "entt.hpp"
#include "systems/EquipmentSystem.hpp"
#include "Components.hpp"
#include "EntityFactory.hpp"
#include "Cave.hpp"
#include "Utils.hpp"
#include "Dice.hpp"

void EntityFactory::init()
{
	read_definitions(items_path);
	read_definitions(fungi_path);
	read_definitions(creatures_path);
}

void EntityFactory::read_definitions(const char* path)
{
	std::ifstream file(path);
	if (!file.is_open())
		throw std::runtime_error(std::string("Opening file failed: ") + path);
	nlohmann::json definitions;
	file >> definitions;
	file.close();
	add_entities(definitions);
}

using FieldParser = std::function<void(entt::registry&, entt::entity, const nlohmann::json&)>;

std::unordered_map<std::string, FieldParser> field_parsers =
{
	{ "Name", [](auto& p, auto e, const nlohmann::json& data)
		{
			p.template emplace<Name>(e, data.get<std::string>());
		}
	},
	{ "Renderable", [](auto& p, auto e, const nlohmann::json& data)
		{	// Can be rendered
			std::string glyph_str = data["glyph"].get<std::string>();
			wchar_t glyph = static_cast<wchar_t>(glyph_str[0]);
			auto& c = data["color"];
			Color color(c[0].get<int>(), c[1].get<int>(), c[2].get<int>());
			p.template emplace<Renderable>(e, glyph, color);
		}
	},
	{ "Solid", [](auto& p, auto e, const nlohmann::json& data)
		{	// Cannot move through or be moved through by another Solid entity
			(void) data;
			p.template emplace<Solid>(e);
		}
	},
	{ "Opaque", [](auto& p, auto e, const nlohmann::json& data)
		{	// Cannot been seen through
			(void) data;
			p.template emplace<Opaque>(e);
		}
	},
	{ "Inventory", [](auto& p, auto e, const nlohmann::json& data)
		{	// Has an inventory and maybe starting items
			std::vector<entt::entity> items;
			for (const auto& item : data)
			{
				if (!item.contains("id"))
				{
					Log::log("Item is missing id: " + item.dump(4));
					continue;
				}
				auto i = EntityFactory::instance().create_entity(p, item["id"].get<std::string>());
				items.push_back(i);
			}
			p.template emplace<Inventory>(e, items);
		}
	},
	{ "Vision", [](auto& p, auto e, const nlohmann::json& data)
		{	// Can see and has a vision range
			p.template emplace<Vision>(e, data.get<int>());
		}
	},
	{ "Glow", [](auto& p, auto e, const nlohmann::json& data)
		{	// Emits light around it
			p.template emplace<Glow>(e, data["strength"].get<double>(), data["radius"].get<double>());
		}
	},
	{ "Player", [](auto& p, auto e, const nlohmann::json& data)
		{	// Is the player, one and only
			(void) data;
			p.template emplace<Player>(e);
		}
	},
	{ "Damage", [](auto& p, auto e, const nlohmann::json& data)
		{	// Can deal damage
			p.template emplace<Damage>(e, Dice(data.get<std::string>()));
		}
	},
	{ "Weight", [](auto& p, auto e, const nlohmann::json& data)
		{	// Has a weight in kilograms
			p.template emplace<Weight>(e, data.get<double>());
		}
	},
	{ "Equippable", [](auto& p, auto e, const nlohmann::json& data)
		{	// This entity can be equipped by someone
			EquipmentSystem::Slot slot = EquipmentSystem::parse_slot(data.get<std::string>());
			p.template emplace<Equippable>(e, slot);
		}
	},
	{ "Equipment", [](auto& p, auto e, const nlohmann::json& data)
		{	// This entity has equipment slots and can equip items
			(void) data;
			p.template emplace<Equipment>(e);
		}
	}
};

void EntityFactory::add_entities(nlohmann::json& json)
{
	for (const auto& [name, data] : json.items())
	{
		if (!data.contains("Name"))
			data["Name"] = name;
		LUT[name] = data;
	}
}

entt::entity EntityFactory::create_entity(entt::registry& registry, const std::string& name, Cell* cell)
{
	if (LUT.find(name) == LUT.end())
		throw std::runtime_error("Entity not found: " + name);

	auto entity = registry.create();
	const auto& j = LUT[name];
	for (const auto& [field_name, field_data] : j.items())
	{
		auto it = field_parsers.find(field_name);
		if (it == field_parsers.end())
			throw std::runtime_error("Unknown field name: " + field_name);

		it->second(registry, entity, field_data);
	}
	if (cell != nullptr)
		registry.emplace<Position>(entity, cell);
	return entity;
}

