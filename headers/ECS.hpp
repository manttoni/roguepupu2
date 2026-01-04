#pragma once

#include <string>    // for string
#include "entt.hpp"  // for entity, registry, vector, map, size_t
class Cell;
class Color;
class Cave;

#define MELEE_RANGE 1.5

namespace ECS
{
	Color get_color(const entt::registry& registry, const entt::entity entity);
	wchar_t get_glyph(const entt::registry& registry, const entt::entity entity);
	std::string get_name(const entt::registry& registry, const entt::entity entity);
	std::string get_colored_name(const entt::registry& registry, const entt::entity entity);
	std::vector<std::string> get_colored_names(const entt::registry& registry, const std::vector<entt::entity>& items);
	std::map<std::string, std::string> get_info(const entt::registry& registry, const entt::entity entity);
	Cell* get_cell(const entt::registry& registry, const entt::entity entity);
	double distance(const entt::registry& registry, const entt::entity a, const entt::entity b);
	entt::entity get_player(const entt::registry& registry);
	bool can_see(const entt::registry& registry, const entt::entity seer, const entt::entity target);
	std::vector<entt::entity> get_inventory(const entt::registry& registry, const entt::entity entity);
	size_t get_level(const entt::registry& registry, const entt::entity entity);
	int get_fatigue_max(const entt::registry& registry, const entt::entity entity);
	int get_health_max(const entt::registry& registry, const entt::entity entity);
	int get_mana_max(const entt::registry& registry, const entt::entity entity);
	Cave* get_active_cave(const entt::registry& registry);
	entt::entity get_source(const entt::registry& registry, const Cave& cave);
	entt::entity get_sink(const entt::registry& registry, const Cave& cave);
	double get_weight(const entt::registry& registry, const entt::entity entity);
	void destroy_entity(entt::registry& registry, const entt::entity entity);
};
