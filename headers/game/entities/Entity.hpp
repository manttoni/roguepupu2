#pragma once

#include <algorithm>
#include <concepts>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>
#include <nlohmann/json.hpp>

#include "ncurses/Color.hpp"
#include "game/components/Component.hpp"
#include "external/entt/entt.hpp"
#include "game/Enum.hpp"
#include "databases/EntityDatabase.hpp"
#include "game/entities/Definition.hpp"

namespace Game::Entity
{
	using Json = nlohmann::json;

	// Keep the definition visible for compile-time calls.
	inline constexpr bool valid_id(const Definition::ID& id) noexcept
	{
		constexpr std::string_view valid_characters{
			"abcdefghijklmnopqrstuvwxyz0123456789_"
		};

		if (id.empty())
			return false;
		if (id.front() < 'a' || id.front() > 'z')
			return false;
		if (id.back() == '_')
			return false;
		if (id.find("__") != std::string_view::npos)
			return false;

		return id.find_first_not_of(valid_characters) == std::string_view::npos;
	}

	bool valid_entity(const entt::registry& registry, entt::entity entity);
	bool valid_definition(const Json& definition);

	template<typename C>
	bool definition_has_component(const Json& definition, const Json& tags)
	{
		if constexpr (std::derived_from<C, Game::Component::Tag::Base>)
		{
			return std::ranges::any_of(tags, [](const Json& tag)
					{
						return tag.is_string()
						&& tag.get_ref<const std::string&>() == C::string;
					});
		}
		else
		{
			return definition.contains(C::string);
		}
	}

	namespace Detail
	{
		template<typename Component>
		bool matches(
				const entt::registry& registry,
				const entt::entity entity,
				const Component& expected)
		{
			if constexpr (std::is_empty_v<Component>)
			{
				// The calling view has already checked tag presence.
				return true;
			}
			else
			{
				return registry.get<Component>(entity) == expected;
			}
		}
	}

	template<typename... Components>
	std::vector<entt::entity> find_all(
			const entt::registry& registry,
			const Components&... expected)
	{
		static_assert(sizeof...(Components) > 0);

		std::vector<entt::entity> result;
		const auto view = registry.view<const Components...>();

		for (const entt::entity entity : view)
		{
			if ((Detail::matches(registry, entity, expected) && ...))
				result.push_back(entity);
		}

		return result;
	}
	template<typename T>
	struct is_vector : std::false_type {};

	template<typename T, typename Allocator>
	struct is_vector<std::vector<T, Allocator>> : std::true_type {};

	template<typename T>
	inline constexpr bool is_vector_v = is_vector<T>::value;

	template<typename T>
	T parse_value(const Json& data)
	{
		if constexpr (Game::Enum::GameEnum<T>)
		{
			return Game::Enum::from_string<T>(data.get<std::string>());
		}
		else if constexpr (is_vector_v<T>)
		{
			T values;
			values.reserve(data.size());

			for (const auto& element : data)
				values.push_back(parse_value<typename T::value_type>(element));

			return values;
		}
		else if constexpr (std::same_as<T, char>)
		{
			return data.get<std::string>()[0];
		}
		else
		{
			return data.get<T>();
		}
	}

	entt::entity create(entt::registry& registry, const Definition& definition);
	entt::entity create(entt::registry& registry, const EntityDatabase& entity_database, const Definition::ID& id);

	bool requires_component(const Definition& definition, const std::string& component);
} // namespace Game::Entity

