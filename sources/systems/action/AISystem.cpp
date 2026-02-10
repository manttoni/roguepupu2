#include "components/Components.hpp"
#include "domain/Attack.hpp"
#include "domain/Intent.hpp"
#include "domain/Position.hpp"
#include "external/entt/entt.hpp"
#include "systems/action/AISystem.hpp"
#include "systems/combat/AttackSystem.hpp"
#include "systems/perception/VisionSystem.hpp"
#include "systems/position/MovementSystem.hpp"
#include "systems/state/AlignmentSystem.hpp"
#include "systems/state/EquipmentSystem.hpp"
#include "utils/ECS.hpp"
#include "utils/Random.hpp"

namespace AISystem
{
	bool will_engage_enemy(const entt::registry& registry, Intent& intent)
	{
		if (!intent.actor.position.is_valid())
			return false;

		auto visible_entities = VisionSystem::get_visible_entities(registry, intent.actor.entity);
		std::sort(visible_entities.begin(), visible_entities.end(),
				[&](const auto a, const auto b)
				{
				return ECS::distance(registry, a, intent.actor.entity) < ECS::distance(registry, b, intent.actor.entity);
				});
		for (const auto entity : visible_entities)
		{
			if (!registry.any_of<Alignment>(entity))
				continue;
			if (AlignmentSystem::is_hostile(registry, intent.actor.entity, entity))
			{
				const auto distance = ECS::distance(registry, intent.actor.entity, entity);
				auto attacks = AttackSystem::get_attacks_all_loadouts(registry, intent.actor.entity);
				std::sort(attacks.begin(), attacks.end(),
						[&](const auto& a, const auto& b)
						{ // its possible to check weapon modifiers here also, but they dont exist now
						const auto& [a_weapon, a_attack] = a;
						const auto& [b_weapon, b_attack] = b;
						const auto a_damage = AttackSystem::get_attack_damage(registry, intent.actor.entity, *a_attack);
						const auto b_damage = AttackSystem::get_attack_damage(registry, intent.actor.entity, *b_attack);
						return a_damage > b_damage;
						});
				for (const auto& weapon_attack : attacks)
				{
					const auto& [weapon, attack] = weapon_attack;
					if (attack->range < distance)
						continue;
					// check fatigue also when implemented
					// they might want to use bow in melee, but currently no neat solution to that
					// after all checks, this is the attack they will use
					// check if the weapon is in other loadout
					if (weapon != entt::null && !EquipmentSystem::is_equipped(registry, intent.actor.entity, weapon))
					{
						intent.type = Intent::Type::SwapLoadout;
						return true;
					}
					intent.target.entity = entity;
					intent.target.position = registry.get<Position>(entity);
					intent.type = Intent::Type::Attack;
					intent.weapon_attack = weapon_attack;
					return true;
				}
				// no attack was suitable, then approach
				intent.target.position = MovementSystem::get_first_step(
						registry,
						intent.actor.position,
						registry.get<Position>(entity));
				intent.type = Intent::Type::Move;
				return true;
			}
		}
		return false;
	}

	bool will_wander(const entt::registry& registry, Intent& intent)
	{
		if (!intent.actor.position.is_valid())
			return false;

		const auto& cave = ECS::get_cave(registry, intent.actor.position);
		auto nearby_positions = cave.get_nearby_positions(intent.actor.position, 1.5);
		if (nearby_positions.empty())
			return false;

		std::shuffle(nearby_positions.begin(), nearby_positions.end(), Random::rng());
		for (const auto position : nearby_positions)
		{
			if (MovementSystem::can_move(registry, intent.actor.position, position))
			{
				intent.target.position = position;
				intent.type = Intent::Type::Move;
				return true;
			}
		}
		return false;
	}

	Intent get_npc_intent(const entt::registry& registry, const entt::entity npc)
	{
		if (!registry.all_of<AI>(npc))
			return {.type = Intent::Type::DoNothing};

		const auto& ai = registry.get<AI>(npc);
		Intent intent;
		intent.actor.entity = npc;
		if (registry.all_of<Position>(npc))
			intent.actor.position = registry.get<Position>(npc);

		if (ai.aggressive == true && will_engage_enemy(registry, intent) == true)
			return intent;

		if (ai.idle_wander == true && will_wander(registry, intent) == true)
			return intent;

		intent.type = Intent::Type::DoNothing;
		return intent;
	}
};
