#include "game/entities/Entity.hpp"

#include "utils/Log.hpp"

namespace Game::Entity
{
	bool valid_entity(const entt::registry& registry, const entt::entity entity)
	{
		if (!registry.valid(entity))
			return false;

#define X(component, dependency) \
		if (registry.all_of<component>(entity) \
			&& !registry.all_of<dependency>(entity)) \
		{ \
			return false; \
		}
#include "game/components/Dependency.def"
#undef X

		return true;
	}

	bool valid_definition(const Json& definition)
	{
		if (!definition.is_object())
		{
			Log::error() << "definition not a json object";
			return false;
		}

		const auto tags_iterator = definition.find("Tags");
		if (tags_iterator == definition.end() || !tags_iterator->is_array())
		{
			Log::error() << "tags invalid";
			return false;
		}

		const auto& tags = *tags_iterator;
		if (!std::ranges::all_of(tags, [](const Json& tag)
				{
					return tag.is_string();
				}))
		{
			Log::error() << "tags must be strings";
			return false;
		}

		std::string missing_dependencies;
#define X(component, dependency) \
		if (definition_has_component<component>(definition, tags) \
			&& !definition_has_component<dependency>(definition, tags)) \
		{ \
			missing_dependencies += "\n[" + std::string(#dependency) + "]"; \
		}
#include "game/components/Dependency.def"
#undef X

		if (missing_dependencies.empty())
			return true;

		Log::error() << "Missing dependencies: " << missing_dependencies;
		return false;
	}
	/* Entity Creation
	 * uses macro expansions from game/components/ .def files
	 * */
	bool emplace_component(
			entt::registry& registry,
			const entt::entity entity,
			const std::string& component_str,
			const Json& data)
	{
		if (component_str.empty())
			return false;
#define X(name, type) \
		else if (component_str == #name) \
		{ \
			registry.emplace<Component::Value::name>(entity, parse_value<type>(data)); \
		}
#include "game/components/Value.def"
#undef X
#define X(name, type) \
		else if (component_str == #name) \
		{ \
			registry.emplace<Component::List::name>(entity, parse_value<std::vector<type>>(data)); \
		}
#include "game/components/List.def"
#undef X
#define X(name, type) \
		else if (component_str == #name) \
		{ \
			registry.emplace<Component::Resource::name>(entity, parse_value<type>(data)); \
		}
#include "game/components/Resource.def"
#undef X
		else
		{
			Log::warning() << "No component called \"" << component_str << "\" exists";
			return false; // There is no matching component
		}
		Log::debug() << "Emplaced component: " << component_str;
		return true;
	}
	bool emplace_tags(
			entt::registry& registry,
			const entt::entity entity,
			const std::vector<std::string>& tags
			)
	{
		for (const auto& tag : tags)
		{
			if (tag.empty())
				return false;
#define X(name) \
			else if (tag == #name) \
			registry.emplace<Component::Tag::name>(entity);
#include "game/components/Tag.def"
#undef X
			else
			{
				Log::warning() << "No tag called \"" << tag << "\" exists";
				return false;
			}
			Log::debug() << "Emplaced tag: " << tag;
		}
		return true;
	}
	entt::entity create(entt::registry& registry, const Definition& definition)
	{
		const auto entity = registry.create();
		for (const auto& [component_str, data] : definition.data.items())
		{
			if (component_str == "Tags")
				emplace_tags(registry, entity, data.get<std::vector<std::string>>());
			else
				emplace_component(registry, entity, component_str, data);
		}
		return entity;
	}

	entt::entity create(entt::registry& registry, const EntityDatabase& entity_database, const Definition::ID& id)
	{
		if (!entity_database.definitions.contains(id))
		{
			throw std::runtime_error("Entity Definition ID \'" + id + "\' not found");
			return entt::null;
		}
		const Definition definition{id, entity_database.definitions.at(id)};
		return create(registry, definition);
	}

} // namespace Game::Entity

