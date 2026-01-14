// This will eventually get deleted
#pragma once

#include <vector>
#include <map>
#include <string>    // for string
#include "entt.hpp"  // for entity, registry, vector, map, size_t
#include "Cave.hpp"
#include "Cell.hpp"
#include "Color.hpp"
#include "Components.hpp"

#define MELEE_RANGE 1.5

namespace ECS
{
	Cave& get_active_cave(entt::registry& registry);
	Color get_color(const entt::registry& registry, const entt::entity entity);
	const Cave& get_active_cave(const entt::registry& registry);
	double get_size(const entt::registry& registry, const entt::entity entity);
	double get_weight(const entt::registry& registry, const entt::entity entity);
	entt::entity get_player(const entt::registry& registry);
	entt::registry init_registry();
	int get_fatigue_max(const entt::registry& registry, const entt::entity entity);
	int get_health_max(const entt::registry& registry, const entt::entity entity);
	int get_mana_max(const entt::registry& registry, const entt::entity entity);
	size_t get_level(const entt::registry& registry, const entt::entity entity);
	std::map<std::string, std::string> get_info(const entt::registry& registry, const entt::entity entity);
	std::string get_colored_name(const entt::registry& registry, const entt::entity entity);
	std::string get_name(const entt::registry& registry, const entt::entity entity);

	std::vector<entt::entity> get_entities(const entt::registry& registry, const Position& position);
	std::vector<entt::entity> get_entities(const entt::registry& registry, const size_t cave_idx);

	std::vector<entt::entity> get_inventory(const entt::registry& registry, const entt::entity entity);
	std::vector<std::string> get_colored_names(const entt::registry& registry, const std::vector<entt::entity>& items);
	void destroy_entity(entt::registry& registry, const entt::entity entity);
	wchar_t get_glyph(const entt::registry& registry, const entt::entity entity);
};
