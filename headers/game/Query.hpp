#pragma once

#include <vector>
#include "external/entt/entt.hpp"
#include "game/component/Component.hpp"

namespace Game::Query
{
	inline auto living_actors(entt::registry& registry)
	{
		return registry.view<Component::Tag::Actor>(
			entt::exclude<Component::Tag::Dead>);
	}

	namespace EntityDetail
	{
		template<typename Component>
			bool matches(
				const entt::registry& registry,
				const entt::entity entity,
				const Component& expected)
			{
				if constexpr (std::is_empty_v<Component>)
				{
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
				if ((EntityDetail::matches(registry, entity, expected) && ...))
					result.push_back(entity);
			}

			return result;
		}
}
