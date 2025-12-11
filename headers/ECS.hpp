#pragma once

#include <string>
#include <map>
#include "entt.hpp"
#include "Color.hpp"
#include "Components.hpp"

#define MELEE_RANGE 1.5

namespace ECS
{
	std::string get_category(const entt::registry& registry, const entt::entity entity);
	Color get_rarity_color(const std::string& rarity);
	Color get_color(const entt::registry& registry, const entt::entity entity);
	wchar_t get_glyph(const entt::registry& registry, const entt::entity entity);
	std::string get_name(const entt::registry& registry, const entt::entity entity);
	std::string get_colored_name(const entt::registry& registry, const entt::entity entity);
	std::vector<std::string> get_colored_names(const entt::registry& registry, const std::vector<entt::entity>& items);
	std::string get_proficiency_requirement(const entt::registry& registry, const entt::entity entity);
	size_t get_armor_class(const entt::registry& registry, const entt::entity entity);
	Damage get_damage(const entt::registry& registry, const entt::entity entity);
	Dice get_versatile_dice(const entt::registry& registry, const entt::entity entity);
	bool has_weapon_property(const entt::registry& registry, const entt::entity entity, const std::string& property);
	std::map<std::string, std::string> get_info(const entt::registry& registry, const entt::entity entity);
	Cell* get_cell(const entt::registry& registry, const entt::entity entity);
	size_t get_value(const entt::registry& registry, const entt::entity entity);
	double distance(const entt::registry& registry, const entt::entity a, const entt::entity b);
	entt::entity get_player(const entt::registry& registry);
	bool is_equippable(const entt::registry& registry, const entt::entity entity);
	std::string get_subcategory(const entt::registry& registry, const entt::entity entity);
	bool can_see(const entt::registry& registry, const entt::entity seer, const entt::entity target);
	std::vector<entt::entity> get_inventory(const entt::registry& registry, const entt::entity entity);
};
