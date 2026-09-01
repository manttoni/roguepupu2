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

#include "core/paths.hpp"
#include "database/EntityFactory.hpp"                              // for EntityFactory
#include "domain/Color.hpp"                                      // for Color
#include "domain/Event.hpp"
#include "domain/Position.hpp"
#include "external/entt/entt.hpp"
#include "systems/state/StateSystem.hpp"
#include "utils/ECS.hpp"
#include "utils/Error.hpp"
#include "utils/JsonUtils.hpp"
#include "utils/Log.hpp"
#include "utils/Parser.hpp"
#include "utils/Random.hpp"
#include "components/Component.hpp"

using ComplexParser = std::function<void(entt::registry&, entt::entity, const Json&)>;
using ValueParser = std::function<void(entt::registry&, entt::entity, const Json&)>;
using TagParser = std::function<void(entt::registry&, entt::entity)>;
using ListParser = ValueParser;

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
std::unordered_map<std::string_view, ComplexParser> complex_parsers =
{
	{ "color", [](auto& reg, auto e, const nlohmann::json& data)
		{
			Color color = Parser::parse_color(data);
			reg.template emplace<Color>(e, color);
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
						EntityFactory::instance().create_entities(reg, data.get<std::vector<std::string>>()) \
						); \
			} \
			reg.template emplace<Component::List::name>(e, \
					data.get<std::vector<type>>() \
					); \
		} \
	},
#include "components/List.def"
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
#include "components/Value.def"
#undef X
};

std::unordered_map<std::string_view, TagParser> tag_parsers =
{
#define X(name) \
	{ #name, [](auto& reg, auto e) \
		{ reg.template emplace<Component::Tag::name>(e); } \
	},
#include "components/Tag.def"
#undef X
};

void EntityFactory::emplace_component(
		entt::registry& registry,
		const entt::entity entity,
		const std::string_view component_name,
		const Json& component_json) const
{
	if (const auto it = complex_parsers.find(component_name);
			it != complex_parsers.end())
	{
		it->second(registry, entity, component_json);
		return;
	}

	if (const auto it = value_parsers.find(component_name);
			it != value_parsers.end())
	{
		it->second(registry, entity, component_json);
		return;
	}

	if (const auto it = tag_parsers.find(component_name);
			it != tag_parsers.end())
	{
		it->second(registry, entity);
		return;
	}

	Error::fatal(
			"No parser exists for component \"" +
			std::string(component_name) +
			"\"");
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
		ECS::queue_event(
				registry,
				SpawnEvent{
				.entity = entity,
				.position = *position
				});
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

