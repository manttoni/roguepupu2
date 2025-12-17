#include <vector>
#include "Components.hpp"
#include "entt.hpp"
#include "systems/CombatSystem.hpp"
#include "systems/DamageSystem.hpp"
#include "systems/EquipmentSystem.hpp"
#include "systems/StatsSystem.hpp"
#include "ECS.hpp"

namespace CombatSystem
{
	int get_fatigue_factor(const entt::registry& registry, const entt::entity entity)
	{
		const int current = registry.get<Resources>(entity).fatigue;
		const int max = ECS::get_fatigue_max(registry, entity);
		return std::max(1, current) / std::max(1, max);
	}
	int get_accuracy(const entt::registry& registry, const entt::entity attacker, const entt::entity weapon)
	{
		const int dexterity = static_cast<int>(registry.get<Attributes>(attacker).dexterity);
		const int weapon_accuracy = static_cast<int>(ECS::get_accuracy(registry, weapon));
		const int fatigue_factor = get_fatigue_factor(registry, attacker);
		const int result = dexterity * std::max(1, weapon_accuracy) * fatigue_factor;
		return static_cast<int>(result);
	}
	int get_equipment_evasion(const entt::registry& registry, const entt::entity defender)
	{
		const auto& equipment = registry.get<Equipment>(defender);
		const int body_armor = ECS::get_evasion(registry, equipment.armor);
		return body_armor;
	}
	int get_equipment_barrier(const entt::registry& registry, const entt::entity defender)
	{
		const auto& equipment = registry.get<Equipment>(defender);
		const int body_armor = ECS::get_barrier(registry, equipment.armor);
		return body_armor;
	}

	int get_evasion(const entt::registry& registry, const entt::entity defender)
	{
		const int dexterity = static_cast<int>(registry.get<Attributes>(defender).dexterity);
		const int equipment_evasion = static_cast<int>(get_equipment_evasion(registry, defender));
		const int fatigue_factor = get_fatigue_factor(registry, defender);
		const int result = std::max(1, dexterity) * std::max(1, equipment_evasion) * fatigue_factor;
		return static_cast<int>(result);
	}

	int get_hit_quality(const entt::registry& registry, const entt::entity attacker, const entt::entity weapon, const entt::entity defender)
	{
		const int accuracy = get_accuracy(registry, attacker, weapon);
		const int evasion = get_evasion(registry, defender);
		return accuracy / evasion;
	}

	int get_base_damage(const entt::registry& registry, const entt::entity attacker, const entt::entity weapon)
	{
		const int strength = static_cast<int>(registry.get<Attributes>(attacker).strength);
		const int weapon_damage_min = ECS::get_damage_min(registry, weapon);
		const int weapon_damage_max = ECS::get_damage_max(registry, weapon);
		const int rand_damage = static_cast<int>(Random::randint(weapon_damage_min, weapon_damage_max));
		return strength * rand_damage;
	}

	int get_barrier(const entt::registry& registry, const entt::entity defender)
	{
		const int intelligence = static_cast<int>(registry.get<Attributes>(defender).intelligence);
		const int equipment_barrier = static_cast<int>(get_equipment_barrier(registry, defender));
		const int result = intelligence * std::max(1, equipment_barrier);
		return result;
	}

	int get_power(const entt::registry& registry, const entt::entity attacker, const entt::entity weapon)
	{
		const int intelligence = registry.get<Attributes>(attacker).intelligence;
		const int weapon_power = ECS::get_power(registry, weapon);
		return intelligence * std::max(1, weapon_power);
	}

	int get_armor(const entt::registry& registry, const entt::entity defender)
	{
		const auto& equipment = registry.get<Equipment>(defender);
		const int body_armor = ECS::get_armor(registry, equipment.armor);
		return body_armor;
	}

	void weapon_attack(entt::registry& registry, const entt::entity attacker, const entt::entity weapon, const entt::entity defender)
	{
		const int base_damage = get_base_damage(registry, attacker, weapon);
		const int hit_quality = get_hit_quality(registry, attacker, weapon, defender);
		const int barrier = get_barrier(registry, defender);
		const int power = get_power(registry, attacker, weapon);
		const int armor = get_armor(registry, defender);
		const int armor_penetration = ECS::get_armor_penetration(registry, weapon);
		const int barrier_efficiency = std::max(0, barrier - power);
		const int armor_deflection = std::max(0, armor - armor_penetration);
		const int damage_taken = (base_damage - barrier_efficiency) * hit_quality - armor_deflection;
		DamageSystem::take_damage(registry, defender, damage_taken);
	}

	void attack(entt::registry& registry, const entt::entity attacker, const entt::entity defender)
	{
		const auto& equipment = registry.get<Equipment>(attacker);
		const auto right_weapon = equipment.right_hand;
		const auto left_weapon = equipment.left_hand;
		if (right_weapon != entt::null)
		{
			weapon_attack(registry, attacker, right_weapon, defender);
		}
		if (left_weapon != entt::null && left_weapon != right_weapon)
		{
			weapon_attack(registry, attacker, left_weapon, defender);
		}
	}
};
