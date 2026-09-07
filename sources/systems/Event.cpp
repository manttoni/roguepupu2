#include <deque>
#include <variant>

#include "external/entt/entt.hpp"
#include "domain/Event.hpp"
#include "utils/Log.hpp"
#include "utils/ECS.hpp"


namespace System::Event

{
	void handle(entt::registry&, const Domain::Event::Null&)
	{
	}
	void handle(entt::registry& registry, const Domain::Event::Move& event)
	{
		Log::debug() << ECS::get_name(registry, event.entity) << " moves";
	}

	void handle(entt::registry& registry, const Domain::Event::Attack& event)
	{
		ECS::game_log(registry) << event.attacker << " attacks " << event.defender << " with " << event.weapon;
	}

	void handle(entt::registry& registry, const Domain::Event::AttackHit& event)
	{
		const auto with = event.ammo == entt::null ? event.weapon : event.ammo;
		ECS::game_log(registry) << event.attacker << " hits " << event.defender << " with " << with;
	}

	void handle(entt::registry& registry, const Domain::Event::AttackMiss& event)
	{
		const auto with = event.ammo == entt::null ? event.weapon : event.ammo;
		ECS::game_log(registry) << event.attacker << " hits " << event.defender << " with " << with;
	}

	void handle(entt::registry& registry, const Domain::Event::TakeDamage& event)
	{
		ECS::game_log(registry) << event.target << " takes " << event.amount << " " << event.damage_type << " damage";
	}

	void handle(entt::registry& registry, const Domain::Event::Spawn& event)
	{
		Log::debug() << ECS::get_name(registry, event.entity) << " spawned";
	}

	void handle(entt::registry& registry, const Domain::Event::Destroy& event)
	{
		Log::debug() << ECS::get_name(registry, event.entity) << " was destroyed";
	}

	void handle(entt::registry& registry, const Domain::Event::Drop& event)
	{
		ECS::game_log(registry) << event.entity << " drops " << event.item;
	}

	void handle(entt::registry& registry, const Domain::Event::Take& event)
	{
		ECS::game_log(registry) << event.entity << " takes " << event.item;
	}

	void handle(entt::registry& registry, const Domain::Event::Equip& event)
	{
		ECS::game_log(registry) << event.entity << " equips " << event.equipment;
	}

	void handle(entt::registry& registry, const Domain::Event::Unequip& event)
	{
		ECS::game_log(registry) << event.entity << " unequips " << event.equipment;
	}

	void handle(entt::registry& registry, const Domain::Event::DiceRoll& event)
	{
		ECS::game_log(registry) << "Rolled dice: " << event.dice << " against DC: " << event.difficulty;
	}
	void handle(entt::registry& registry, const Domain::Event::Death& event)
	{
		ECS::game_log(registry) << event.entity << " dies";
	}
	void handle(entt::registry& registry, const Domain::Event::BecomeHostile& event)
	{
		ECS::game_log(registry) << event.entity << " becomes hostile toward " << event.target;
	}
	void handle(entt::registry& registry, const Domain::Event::ReceiveItem& event)
	{
		ECS::game_log(registry) << event.entity << " receives " << event.item;
	}

	void resolve_events(entt::registry& registry)
	{
		auto& queue = registry.ctx().get<EventQueue>().queue;
		while (!queue.empty())
		{
			Domain::Event::Any event = std::move(queue.front());
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
