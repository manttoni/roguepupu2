#pragma once

#include <cstddef>
#include "entt.hpp"

namespace StatsSystem
{
	enum class Injury
	{
		None,
		Uninjured,
		BarelyInjured,
		Injured,
		BadlyInjured,
		NearDeath,
		Dead
	};
	Injury get_injury(const entt::registry& registry, const entt::entity entity);
	Injury parse_injury(const std::string& injury);
	int get_modifier(const size_t attribute);
	int get_modifier(const entt::registry& registry, const entt::entity, const std::string attribute);
	size_t get_AC(const entt::registry& registry, const entt::entity entity);
};
