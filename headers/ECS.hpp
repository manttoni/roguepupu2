#pragma once

#include <string>
#include "entt.hpp"
#include "Color.hpp"

namespace ECS
{
	Color get_rarity_color(const std::string& rarity);
	Color get_color(const entt::registry& registry, const entt::entity entity);
	std::string get_name(const entt::registry& registry, const entt::entity entity);
	std::string get_description(const entt::registry& registry, const entt::entity entity);
};
