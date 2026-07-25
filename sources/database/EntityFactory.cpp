#include <algorithm>
#include <codecvt>
#include <filesystem>                                     // for path
#include <functional>
#include <limits>
#include <locale>
#include <map>
#include <nlohmann/detail/iterators/iter_impl.hpp>
#include <nlohmann/detail/iterators/iteration_proxy.hpp>
#include <nlohmann/json.hpp>                              // for basic_json
#include <nlohmann/json_fwd.hpp>
#include <optional>
#include <stddef.h>
#include <string>                                         // for string, ope...
#include <unordered_map>
#include <utility>
#include <vector>

#include "components/Components.hpp"                                 // for Resources
#include "core/paths.hpp"
#include "database/EntityFactory.hpp"                              // for EntityFactory
#include "domain/Ability.hpp"
#include "domain/Alignment.hpp"
#include "domain/Color.hpp"                                      // for Color
#include "domain/Effect.hpp"
#include "domain/Event.hpp"
#include "domain/Position.hpp"
#include "domain/Target.hpp"
#include "external/entt/entity/fwd.hpp"
#include "external/entt/entt.hpp"
#include "systems/items/LootSystem.hpp"
#include "systems/state/StateSystem.hpp"
#include "utils/ECS.hpp"
#include "utils/Error.hpp"
#include "utils/JsonUtils.hpp"
#include "utils/Log.hpp"
#include "utils/Parser.hpp"
#include "utils/Random.hpp"

using Json = nlohmann::json;
using ComplexParser = std::function<void(entt::registry&, entt::entity, const Json&)>;
using ValueParser = std::function<void(entt::registry&, entt::entity, const Json&)>;
using TagParser = std::function<void(entt::registry&, entt::entity)>;

std::unordered_map<std::string_view, ComplexParser> complex_parsers =
{
	{ "color", [](auto& reg, auto e, const nlohmann::json& data)
		{
			Color color = Parser::parse_color(data);
			reg.template emplace<Color>(e, color);
		}
	},
	{ "alignment", [](auto& reg, auto e, const nlohmann::json& data)
		{
			Alignment alignment;
			alignment.tolerance = data["tolerance"].get<double>();
			alignment.chaos_law = data["chaos_law"].get<double>();
			alignment.evil_good = data["evil_good"].get<double>();
			reg.template emplace<Alignment>(e, alignment);
		}
	},
	{ "dice", [](auto& reg, auto e, const nlohmann::json& data)
		{
			Dice dice = Parser::parse_dice(data);
			reg.template emplace<Dice>(e, dice);
		}
	}
};

std::unordered_map<std::string_view, ListParser> list_parsers =
{
#define X(name, type) \
	{ #name, [](auto& reg, auto e, const Json& data) \
		{ \
			if constexpr (std::same_as<type, entt::entity>) \
			{ \
				reg.template emplace<Component::List::name>(e, \
						create_entities(reg, data.get<std::vector<std::string>>()) \
						); \
			} \
			reg.template emplace<Component::List::name>(e, \
					data.get<std::vector<type>>() \
					); \
		} \
	},

	LIST_COMPONENTS(X)
#undef X
};

std::unordered_map<std::string_view, ValueParser> value_parsers =
{
#define X(name, type) \
	{ #name, [](auto& reg, auto e, const Json& data) \
		{ \
			reg.template emplace<Component::Value::name>(e, data.get<type>()); \
		} \
	},

	VALUE_COMPONENTS(X)
#undef X
};

std::unordered_map<std::string_view, TagParser> tag_parsers =
{
#define X(name) \
	{ #name, [](auto& reg, auto e) \
		{ reg.template emplace<Component::Tag::name>(e); } \
	},

	TAG_COMPONENTS(X)
#undef X
};



void EntityFactory::init()
{
	const std::filesystem::path file = "data/entities.json";
	nlohmann::json definitions = Parser::read_json_file(file);
	add_entities(definitions);
	Log::info() << "Entities parsed";
}

void EntityFactory::add_entities(nlohmann::json& entities)
{
	for (const auto& entity : entities)
		LUT[entity["name"].get<std::string>()] = entity;
}

void EntityFactory::emplace_component(entt::registry& registry, const entt::entity entity, const std::string_view component_name, const Json& component_json)
{
	if (ignored_component(component_name))
		return;

	auto it = complex_parsers.find(component_name);
	if (it != complex_parsers.end())
	{
		it->second(registry, entity, component_data);
		return;
	}

	it = value_parsers.find(component_name);
	if (it != value_parsers.end())
	{
		it->second(registry, entity, component_data);
		return;
	}

	it = tag_parsers.find(component_name);
	if (it != tag_parsers.end())
	{
		it->second(registry, entity, component_data);
		return;
	}

	Error::fatal("No parser exists for component \"" + component_name + "\"");
}

entt::entity EntityFactory::create_entity(entt::registry& registry, const std::string& name, const std::optional<Position>& position) const
{
	if (LUT.find(name) == LUT.end())
		Error::fatal("Entity not found: " + name);

	const entt::entity entity = registry.create();
	const auto& entity_json = LUT.at(name);
	for (const auto& [component_name, component_json] : entity_json.items())
		emplace_component(registry, entity, component_name, component_json);

	if (position.has_value())
	{
		registry.emplace<Position>(entity, *position);
		Event spawn_event;
		spawn_event.type = Event::Type::Spawn;
		spawn_event.target.entity = entity;
		spawn_event.target.position = *position;
		ECS::queue_event(registry, spawn_event);
	}

	return entity;
}

std::vector<entt::entity> EntityFactory::create_entities(entt::registry& registry, const nlohmann::json& filters) const
{
	const auto entity_ids = filter_entity_ids(filters);
	return create_entities(registry, entity_ids);
}

std::vector<entt::entity> EntityFactory::create_entities(entt::registry& registry, const std::vector<std::string>& entity_ids) const
{
	std::vector<entt::entity> entities;
	entities.reserve(entity_ids.size());
	for (const auto& id : entity_ids)
	{
		const auto entity = create_entity(registry, id);
		entities.push_back(entity);
	}
	return entities;
}

std::vector<entt::entity> EntityFactory::create_entities(entt::registry& registry, const std::string& id, const size_t amount) const
{
	std::vector<entt::entity> entities;
	for (size_t i = 0; i < amount; ++i)
	{
		const auto e = create_entity(registry, id);
		entities.push_back(e);
	}
	return entities;
}

std::vector<std::string> EntityFactory::filter_entity_ids(const nlohmann::json& filters) const
{
	std::vector<std::string> ids;
	for (const auto& [id, data] : LUT)
	{
		bool include = true;
		for (const auto& [filter_type, filter_data] : filters.items())
		{
			if (filter_type == "contains_all" && !JsonUtils::contains_all(data, filter_data))
				include = false;
			else if (filter_type == "contains_any" && !JsonUtils::contains_any(data, filter_data))
				include = false;
			else if (filter_type == "contains_none" && !JsonUtils::contains_none(data, filter_data))
				include = false;

			if (include == false) break;
		}
		if (include)
			ids.push_back(id);
	}
	return ids;
}

