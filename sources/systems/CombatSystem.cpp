#include <vector>
#include <chrono>
#include <thread>
#include "Components.hpp"
#include "entt.hpp"
#include "systems/ActionSystem.hpp"
#include "systems/CombatSystem.hpp"
#include "systems/DamageSystem.hpp"
#include "systems/EquipmentSystem.hpp"
#include "ECS.hpp"
#include "GameLogger.hpp"
#include "Utils.hpp"

namespace CombatSystem
{
	double get_fatigue_factor(const entt::registry& registry, const entt::entity entity)
	{
		const double current = static_cast<double>(registry.get<Resources>(entity).fatigue);
		const double max = static_cast<double>(ECS::get_fatigue_max(registry, entity));
		return Random::randreal(1.0, current) / Random::randreal(1, max);
	}
	double get_accuracy(const entt::registry& registry, const entt::entity attacker, const entt::entity weapon)
	{
		const double dexterity = static_cast<double>(registry.get<Attributes>(attacker).dexterity);
		const double weapon_accuracy = static_cast<double>(ECS::get_accuracy(registry, weapon));
		const double fatigue_factor = get_fatigue_factor(registry, attacker);
		const double result = dexterity * std::max(1.0, weapon_accuracy) * fatigue_factor;
		return static_cast<double>(result);
	}
	double get_equipment_evasion(const entt::registry& registry, const entt::entity defender)
	{
		const auto& equipment = registry.get<Equipment>(defender);
		const double body_armor = ECS::get_evasion(registry, equipment.armor);
		return body_armor;
	}
	double get_equipment_barrier(const entt::registry& registry, const entt::entity defender)
	{
		const auto& equipment = registry.get<Equipment>(defender);
		const double body_armor = ECS::get_barrier(registry, equipment.armor);
		return body_armor;
	}

	double get_evasion(const entt::registry& registry, const entt::entity defender)
	{
		const double dexterity = static_cast<double>(registry.get<Attributes>(defender).dexterity);
		const double equipment_evasion = static_cast<double>(get_equipment_evasion(registry, defender));
		const double fatigue_factor = get_fatigue_factor(registry, defender);
		const double result = std::max(1.0, dexterity) * std::max(1.0, equipment_evasion) * fatigue_factor;
		return static_cast<double>(result);
	}

	double get_hit_quality(const entt::registry& registry, const entt::entity attacker, const entt::entity weapon, const entt::entity defender)
	{
		const double accuracy = get_accuracy(registry, attacker, weapon);
		const double evasion = get_evasion(registry, defender);
		return Random::randreal(1, accuracy) / Random::randreal(1, evasion);
	}

	double get_base_damage(const entt::registry& registry, const entt::entity attacker, const entt::entity weapon)
	{
		const double strength = static_cast<double>(registry.get<Attributes>(attacker).strength);
		const double weapon_damage_min = ECS::get_damage_min(registry, weapon);
		const double weapon_damage_max = ECS::get_damage_max(registry, weapon);
		const double rand_damage = Random::randreal(weapon_damage_min, weapon_damage_max);
		const double dual_wield_multiplier = EquipmentSystem::is_dual_wielding(registry, attacker) ? 0.75 : 1.0;
		return strength * rand_damage * dual_wield_multiplier;
	}

	double get_barrier(const entt::registry& registry, const entt::entity defender)
	{
		const double intelligence = static_cast<double>(registry.get<Attributes>(defender).intelligence);
		const double equipment_barrier = static_cast<double>(get_equipment_barrier(registry, defender));
		const double result = intelligence * std::max(1.0, equipment_barrier);
		return result;
	}

	double get_power(const entt::registry& registry, const entt::entity attacker, const entt::entity weapon)
	{
		const double intelligence = registry.get<Attributes>(attacker).intelligence;
		const double weapon_power = ECS::get_power(registry, weapon);
		return intelligence * std::max(1.0, weapon_power);
	}

	double get_equipment_armor(const entt::registry& registry, const entt::entity defender)
	{
		const auto& equipment = registry.get<Equipment>(defender);
		const double body_armor = ECS::get_armor(registry, equipment.armor);
		return body_armor;
	}

	double get_armor(const entt::registry& registry, const entt::entity defender)
	{
		const double strength = registry.get<Attributes>(defender).strength;
		const double equipment_armor = get_equipment_armor(registry, defender);
		return strength * equipment_armor;
	}

	void weapon_attack(entt::registry& registry, const entt::entity attacker, const entt::entity weapon, const entt::entity defender)
	{
		if (registry.get<Resources>(defender).health <= 0)
			return;
		registry.ctx().get<GameLogger>().log(
				ECS::get_colored_name(registry, attacker) + " attacks " +
				ECS::get_colored_name(registry, defender) + " with " +
				ECS::get_colored_name(registry, weapon));

		const double hit_quality = std::min(1.0, get_hit_quality(registry, attacker, weapon, defender));
		if (hit_quality < 0.5)
		{
			registry.ctx().get<GameLogger>().log("The attack misses");
			return;
		}
		const double base_damage = get_base_damage(registry, attacker, weapon);
		const double barrier = get_barrier(registry, defender);
		const double power = get_power(registry, attacker, weapon);
		const double barrier_efficiency = std::max(0.0, barrier - power);
		if (barrier_efficiency >= base_damage)
		{
			registry.ctx().get<GameLogger>().log("The attack is repelled");
			return;
		}
		const double armor = get_armor(registry, defender);
		const double armor_penetration = ECS::get_armor_penetration(registry, weapon);
		const double armor_deflection = std::max(0.0, armor - armor_penetration);
		const double damage_taken = (base_damage - barrier_efficiency) * hit_quality - armor_deflection;
		const int final_damage = std::max(0, static_cast<int>(std::round(damage_taken)));
		if (final_damage <= 0)
		{
			registry.ctx().get<GameLogger>().log("The attack is deflected");
			return;
		}
		DamageSystem::take_damage(registry, defender, final_damage);
	}

	void attack(entt::registry& registry, const entt::entity attacker, const entt::entity defender)
	{
		const auto& equipment = registry.get<Equipment>(attacker);
		const auto right_weapon = equipment.right_hand;
		const auto left_weapon = equipment.left_hand;
		if (right_weapon != entt::null)
		{
			weapon_attack(registry, attacker, right_weapon, defender);
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
		if (left_weapon != entt::null && left_weapon != right_weapon)
		{
			weapon_attack(registry, attacker, left_weapon, defender);
		}
		ActionSystem::use_action(registry, attacker);
	}
};
