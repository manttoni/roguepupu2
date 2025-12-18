#pragma once

#include <string>    // for string
#include "entt.hpp"  // for entity, registry, vector, map, size_t
class Cell;
class Color;
struct Damage;

#define MELEE_RANGE 1.5

namespace ECS
{
	Color get_color(const entt::registry& registry, const entt::entity entity);
	wchar_t get_glyph(const entt::registry& registry, const entt::entity entity);
	std::string get_name(const entt::registry& registry, const entt::entity entity);
	std::string get_colored_name(const entt::registry& registry, const entt::entity entity);
	std::vector<std::string> get_colored_names(const entt::registry& registry, const std::vector<entt::entity>& items);
	Damage get_damage(const entt::registry& registry, const entt::entity entity);
	std::map<std::string, std::string> get_info(const entt::registry& registry, const entt::entity entity);
	Cell* get_cell(const entt::registry& registry, const entt::entity entity);
	double distance(const entt::registry& registry, const entt::entity a, const entt::entity b);
	entt::entity get_player(const entt::registry& registry);
	bool is_equippable(const entt::registry& registry, const entt::entity entity);
	bool can_see(const entt::registry& registry, const entt::entity seer, const entt::entity target);
	std::vector<entt::entity> get_inventory(const entt::registry& registry, const entt::entity entity);
	size_t get_level(const entt::registry& registry, const entt::entity entity);
	bool are_enemies(const entt::registry& registry, const entt::entity a, const entt::entity b);
	int get_damage_min(const entt::registry& registry, const entt::entity entity);
	int get_damage_max(const entt::registry& registry, const entt::entity entity);
	int get_armor_penetration(const entt::registry& registry, const entt::entity entity);
	int get_armor(const entt::registry& registry, const entt::entity entity);
	int get_accuracy(const entt::registry& registry, const entt::entity entity);
	int get_evasion(const entt::registry& registry, const entt::entity entity);
	int get_dexterity(const entt::registry& registry, const entt::entity entity);
	int get_strength(const entt::registry& registry, const entt::entity entity);
	double get_crit_multiplier(const entt::registry& registry, const entt::entity entity);
	double get_crit_chance(const entt::registry& registry, const entt::entity entity);
	int get_power(const entt::registry& registry, const entt::entity entity);
	int get_barrier(const entt::registry& registry, const entt::entity entity);
	int get_intelligence(const entt::registry& registry, const entt::entity entity);
	int get_fatigue_max(const entt::registry& registry, const entt::entity entity);
	int get_health_max(const entt::registry& registry, const entt::entity entity);
	int get_mana_max(const entt::registry& registry, const entt::entity entity);
	bool is_dead(const entt::registry& registry, const entt::entity entity);
	bool has_actions_left(const entt::registry& registry, const entt::entity entity);
};
