#include <deque>
#include <variant>

#include "external/entt/entt.hpp"
#include "domain/Event.hpp"
#include "utils/Log.hpp"
#include "utils/ECS.hpp"

namespace EventSystem
{
	void handle(entt::registry&, const NullEvent&)
	{
	}
	void handle(entt::registry& registry, const MoveEvent& event)
	{
		Log::debug() << ECS::get_name(registry, event.entity) << " moves";
	}

	void handle(entt::registry& registry, const AttackEvent& event)
	{
		ECS::game_log(registry) << event.attacker << " attacks " << event.defender << " with " << event.weapon;
	}

	void handle(entt::registry& registry, const AttackHitEvent& event)
	{
		const auto with = event.ammo == entt::null ? event.weapon : event.ammo;
		ECS::game_log(registry) << event.attacker << " hits " << event.defender << " with " << with;
	}

	void handle(entt::registry& registry, const AttackMissEvent& event)
	{
		const auto with = event.ammo == entt::null ? event.weapon : event.ammo;
		ECS::game_log(registry) << event.attacker << " hits " << event.defender << " with " << with;
	}

	void handle(entt::registry& registry, const TakeDamageEvent& event)
	{
		ECS::game_log(registry) << event.target << " takes " << event.amount << " " << event.damage_type << " damage";
	}

	void handle(entt::registry& registry, const SpawnEvent& event)
	{
		Log::debug() << ECS::get_name(registry, event.entity) << " spawned";
	}

	void handle(entt::registry& registry, const DestroyEvent& event)
	{
		Log::debug() << ECS::get_name(registry, event.entity) << " was destroyed";
	}

	void handle(entt::registry& registry, const DropEvent& event)
	{
		ECS::game_log(registry) << event.entity << " drops " << event.item;
	}

	void handle(entt::registry& registry, const TakeEvent& event)
	{
		ECS::game_log(registry) << event.entity << " takes " << event.item;
	}

	void handle(entt::registry& registry, const EquipEvent& event)
	{
		ECS::game_log(registry) << event.entity << " equips " << event.equipment;
	}

	void handle(entt::registry& registry, const UnequipEvent& event)
	{
		ECS::game_log(registry) << event.entity << " unequips " << event.equipment;
	}

	void handle(entt::registry& registry, const DiceRollEvent& event)
	{
		ECS::game_log(registry) << "Rolled dice: " << event.dice << " against DC: " << event.difficulty;
	}
	void handle(entt::registry& registry, const DeathEvent& event)
	{
		ECS::game_log(registry) << event.entity << " dies";
	}
	void handle(entt::registry& registry, const BecomeHostileEvent& event)
	{
		ECS::game_log(registry) << event.entity << " becomes hostile toward " << event.target;
	}
	void handle(entt::registry& registry, const ReceiveItemEvent& event)
	{
		ECS::game_log(registry) << event.entity << " receives " << event.item;
	}

	void resolve_events(entt::registry& registry)
	{
		auto& queue = registry.ctx().get<EventQueue>().queue;
		while (!queue.empty())
		{
			Event event = std::move(queue.front());
			queue.pop_front();

			std::visit(
					[&](const auto& e)
					{
					handle(registry, e);
					},
					event);
		}
		auto destroyed = registry.view<Component::Tag::Destroyed>();

		const std::vector<entt::entity> entities{
			destroyed.begin(),
				destroyed.end()
		};

		for (const entt::entity entity : entities)
			registry.destroy(entity);
	}
};
