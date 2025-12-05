#include <fstream>
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <unordered_map>
#include <iostream>
#include "entt.hpp"
#include "Components.hpp"
#include "EntityFactory.hpp"
#include "Cave.hpp"
#include "Utils.hpp"

void EntityFactory::init()
{
	read_definitions();
	create_lut();
}

void EntityFactory::read_definitions()
{
	std::ifstream file(path);
	if (!file.is_open())
		throw std::runtime_error("Opening file failed");
	file >> definitions;
	file.close();
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
		{	// Has an inventory / can store entities inside
			(void) data;
			p.template emplace<Inventory>(e);
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
	}
};

void EntityFactory::create_lut()
{
	EntityType type = EntityType{};
	for (const auto& [name, data] : definitions.items())
	{
		data["Name"] = name;
		LUT[type] = data;
		type = static_cast<EntityType>(static_cast<int>(type) + 1 );
	}
}

void EntityFactory::log_prototypes() const
{
	Log::log("Parsed entities:");
	const auto& entities = prototypes.view<Name>();
	for (const auto& entity : entities)
	{
		Log::log(prototypes.get<Name>(entity).name);
	}
	Log::log("Total: " + std::to_string(entities.size()));
}

entt::entity EntityFactory::create_entity(const EntityType type, Cell& cell)
{
	if (LUT.find(type) == LUT.end())
		throw std::runtime_error("Entity not found");

	auto& cave = *cell.get_cave();
	auto& registry = cave.get_registry();
	auto entity = registry.create();

	const auto& j = LUT[type];
	for (const auto& [field_name, field_data] : j.items())
	{
		auto it = field_parsers.find(field_name);
		if (it == field_parsers.end())
			throw std::runtime_error("Unknown field name");

		it->second(registry, entity, field_data);
	}
	registry.emplace<Position>(entity, &cell);

	Log::log("Entity created: " + registry.get<Name>(entity).name);

	return entity;
}

