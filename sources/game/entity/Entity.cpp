#include "game/entity/Entity.hpp"
#include "game/component/Dependency.hpp"
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
#include "game/component/Dependency.def"
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
#include "game/component/Dependency.def"
#undef X

		if (missing_dependencies.empty())
			return true;

		Log::error() << "Missing dependencies: " << missing_dependencies;
		return false;
	}
	/* Entity Creation
	 * uses macro expansions from game/component/ .def files
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
			registry.emplace<Game::Component::Value::name>(entity, parse_value<type>(data)); \
		}
#include "game/component/Value.def"
#undef X
#define X(name, type) \
		else if (component_str == #name) \
		{ \
			registry.emplace<Game::Component::List::name>(entity, parse_value<std::vector<type>>(data)); \
		}
#include "game/component/List.def"
#undef X
#define X(name, type) \
		else if (component_str == #name) \
		{ \
			registry.emplace<Game::Component::Resource::name>(entity, parse_value<type>(data)); \
		}
#include "game/component/Resource.def"
#undef X
		else
		{
			Log::warning() << "No component called \"" << component_str << "\" exists";
			return false; // There is no matching component
		}
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
			registry.emplace<Game::Component::Tag::name>(entity);
#include "game/component/Tag.def"
#undef X
			else
			{
				Log::warning() << "No tag called \"" << tag << "\" exists";
				return false;
			}
		}
		return true;
	}
	entt::entity create(
			entt::registry& registry,
			const Definition& definition)
	{
		const auto entity = registry.create();
		for (const auto& [component_str, data] : definition.data.items())
		{
			if (component_str == "Tags")
				emplace_tags(registry, entity, data.get<std::vector<std::string>>());
			else
				emplace_component(registry, entity, component_str, data);
		}
		const auto* name =
			registry.try_get<Component::Value::Name>(entity);

		Log::debug()
			<< "Entity created: "
			<< (name ? name->value : "<unnamed>");
		return entity;
	}

	entt::entity create(
			entt::registry& registry,
			const EntityDatabase& entity_database,
			const Definition::ID& id)
	{
		if (!entity_database.definitions.contains(id))
		{
			throw std::runtime_error("Entity Definition ID \'" + id + "\' not found");
			return entt::null;
		}
		const Definition definition{id, entity_database.definitions.at(id)};
		return create(registry, definition);
	}

	// Checks only direct requirements and not requirement chains
	bool requires_component(
			const Definition& definition,
			const std::string& component_name)
	{
		const auto tags = definition.data["Tags"].get<std::vector<std::string>>();

		for (const auto& tag : tags)
		{
			const auto required_tags = Component::Dependency::get_required_tag_ids(tag);
			if (std::find(
						required_tags.begin(),
						required_tags.end(),
						component_name) != required_tags.end())
				return true;
			const auto required_non_tags = Component::Dependency::get_required_non_tag_ids(tag);
			if (std::find(
						required_non_tags.begin(),
						required_non_tags.end(),
						component_name) != required_tags.end())
				return true;
		}
		return false;
	}
} // namespace Game::Entity

