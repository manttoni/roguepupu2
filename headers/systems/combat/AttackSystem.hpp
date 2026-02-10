#pragma once

#include "external/entt/fwd.hpp"
#include "domain/Damage.hpp"

struct Attack;
namespace AttackSystem
{
	Damage get_attack_damage(const entt::registry& registry, const entt::entity entity, const Attack& attack);
	double get_attack_hit_chance(const entt::registry& registry, const entt::entity entity, const Attack& attack);
	std::vector<std::pair<entt::entity, const Attack*>> get_attacks_all_loadouts(const entt::registry& registry, const entt::entity entity);
	std::vector<std::pair<entt::entity, const Attack*>> get_attacks(const entt::registry& registry, const entt::entity entity);
	std::pair<entt::entity, const Attack*> get_strongest_melee_attack(const entt::registry& registry, const entt::entity entity);
	std::pair<entt::entity, const Attack*> get_strongest_ranged_attack(const entt::registry& registry, const entt::entity entity);
};
