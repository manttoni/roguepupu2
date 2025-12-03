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
	parse_prototypes();
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

void EntityFactory::parse_prototypes()
{
	EntityType type = EntityType{};
	for (const auto& [name, data] : definitions.items())
	{
		entt::entity e = prototypes.create();
		prototypes.emplace<Name>(e, name);
		for (const auto& [field_name, field_data] : data.items())
		{
			auto it = field_parsers.find(field_name);
			if (it == field_parsers.end())
				throw std::runtime_error("Uknown field name: " + field_name);

			it->second(prototypes, e, field_data);
		}
		LUT[type] = e;
		type = static_cast<EntityType>(static_cast<int>(type) + 1);
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
	auto* cave = cell.get_cave();
	auto& registry = cave->get_registry();

	const auto src = LUT[type];
	const auto dst = registry.create();

	for (auto [id, storage] : prototypes.storage())
	{
		if (storage.contains(src))
			storage.push(dst, storage.value(src));
	}

	return dst;
}

//entt::entity EntityFactory::create_entity(const EntityType type, const entt::entity){}

