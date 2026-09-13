#pragma once

#include <string_view>
#include <nlohmann/json.hpp>

#include "components/Component.hpp"
#include "external/entt/entt.hpp"
#include "utils/Log.hpp"

namespace Entity
{
	using Json = nlohmann::json;
	struct Definition
	{
		std::string id;
		Json data;
	};

	inline constexpr bool valid_id(
			const std::string_view id) noexcept
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

		return id.find_first_not_of(valid_characters) ==
			std::string_view::npos;
	}

	inline bool valid_entity(
			const entt::registry& registry,
			const entt::entity entity)
	{
		if (!registry.valid(entity))
			return false;

#define X(component, dependency)                              \
		if (registry.all_of<component>(entity) &&      \
				!registry.all_of<dependency>(entity))      \
		{                                                         \
			return false;                                         \
		}

#include "components/Dependency.def"
#undef X

		return true;
	}

	template<typename C>
		inline bool definition_has_component(
				const Json& definition,
				const Json& tags)
		{
			if constexpr (
					std::derived_from<C, Component::Tag::Base>)
			{
				return std::ranges::any_of(
						tags,
						[](const Json& tag)
						{
						return tag.is_string() &&
						tag.get_ref<const std::string&>() ==
						C::string;
						});
			}
			else if constexpr (std::same_as<C, Ncurses::Color>)
			{
				return definition.contains("Ncurses::Color");
			}
			else
			{
				return definition.contains(C::string);
			}
		}

	inline bool valid_definition(const Json& definition)
	{
		if (!definition.is_object())
		{
			Log::error() << "definition not a json object";
			return false;
		}

		const auto tags_iterator =
			definition.find("Tags");

		if (tags_iterator == definition.end() ||
				!tags_iterator->is_array())
		{
			Log::error() << "tags invalid";
			return false;
		}

		const auto& tags = *tags_iterator;

		if (!std::ranges::all_of(
					tags,
					[](const Json& tag)
					{
					return tag.is_string();
					}))
		{
			Log::error() << "tags must be strings";
			return false;
		}

		std::string missing_dependencies;
#define X(component, dependency)                              \
		if (definition_has_component<component>(                 \
					definition, tags) &&                             \
					!definition_has_component<dependency>(               \
						definition, tags))                               \
		{                                                        \
			missing_dependencies += "\n[" + std::string(#dependency) + "]"; \
		}

#include "components/Dependency.def"
#undef X

		if (missing_dependencies.empty())
			return true;
		Log::error() << "Missing dependencies: " << missing_dependencies;
		return false;
	}
} // namespace Entity
