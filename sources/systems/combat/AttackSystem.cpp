#include "external/entt/entt.hpp"
#include "systems/combat/AttackSystem.hpp"
#include "systems/state/StateSystem.hpp"
#include "domain/Attack.hpp"

namespace AttackSystem
{
	Damage get_attack_damage(const entt::registry& registry, const entt::entity attacker, const Attack& attack)
	{
		const auto damage_attributes_sum = StateSystem::get_attributes_sum(registry, attacker, attack.damage_attributes);
		Damage calculated = attack.base_damage;
		calculated.amount *= static_cast<size_t>(1.0 + static_cast<double>(damage_attributes_sum) / 100.0);
		return calculated;
	}
	double get_attack_hit_chance(const entt::registry& registry, const entt::entity attacker, const Attack& attack)
	{
		const auto hit_chance_attributes_sum = StateSystem::get_attributes_sum(registry, attacker, attack.hit_chance_attributes);
		return 1.0 + static_cast<double>(hit_chance_attributes_sum) / 100.0;
	}

	std::vector<std::pair<entt::entity, const Attack*>> get_attacks_all_loadouts(const entt::registry& registry, const entt::entity entity)
	{
		auto attacks = get_attacks(registry, entity);
		const auto& equipment_slots = registry.get<EquipmentSlots>(entity);
		const auto& other_loadout = equipment_slots.loadouts[equipment_slots.active_loadout ^ 1];
		if (other_loadout.main_hand != entt::null)
		{
			const auto& main_hand_attacks = registry.get<Attacks>(other_loadout.main_hand).attacks;
			for (const auto& attack : main_hand_attacks)
			{
				const auto pair = std::make_pair(other_loadout.main_hand, &attack);
				auto it = std::find(attacks.begin(), attacks.end(), pair);
				if (it == attacks.end()) // Check this. Reason: same weapon can be in multiple loadouts
					attacks.push_back(pair);
			}
		}
		if (other_loadout.off_hand != entt::null && other_loadout.off_hand != other_loadout.main_hand)
		{
			const auto& off_hand_attacks = registry.get<Attacks>(other_loadout.off_hand).attacks;
			for (const auto& attack : off_hand_attacks)
			{
				const auto pair = std::make_pair(other_loadout.off_hand, &attack);
				auto it = std::find(attacks.begin(), attacks.end(), pair);
				if (it == attacks.end()) // Check this. Reason: same weapon can be in multiple loadouts
					attacks.push_back(pair);
			}
		}
		return attacks;
	}
	std::vector<std::pair<entt::entity, const Attack*>> get_attacks(const entt::registry& registry, const entt::entity entity)
	{
		std::vector<std::pair<entt::entity, const Attack*>> attacks;
		if (registry.all_of<Attacks>(entity))
		{
			for (const auto& attack : registry.get<Attacks>(entity).attacks)
				attacks.push_back(std::make_pair(entt::null, &attack)); // unarmed attacks
		}
		for (const auto& [slot, equipment] : registry.get<EquipmentSlots>(entity).equipped_items)
		{
			if (!registry.all_of<Attacks>(equipment))
				continue;
			for (const auto& attack : registry.get<Attacks>(equipment).attacks)
				attacks.push_back(std::make_pair(equipment, &attack));
		}
		return attacks;
	}
};
