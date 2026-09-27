#pragma once

#include <cstddef>
#include <variant>
#include "game/world/Position.hpp"
#include "external/entt/entt.hpp"
#include "game/Enum.hpp"
#include "game/Dice.hpp"

namespace Game::Event
{
	struct Null
	{
	};
	struct Move
	{
		entt::entity entity = entt::null;
		Game::World::GlobalPosition from{};
		Game::World::GlobalPosition to{};
	};
	struct LeavePosition
	{
		entt::entity entity = entt::null;
		Game::World::GlobalPosition position;
	};
	struct EnterPosition
	{
		entt::entity entity = entt::null;
		Game::World::GlobalPosition position;
	};
	struct LoseMovementPoints
	{
		entt::entity entity = entt::null;
		double amount = 0;
	};
	struct Bump
	{
		entt::entity entity = entt::null;
		Vec2<int> direction{};
	};
	struct Attack
	{
		entt::entity attacker = entt::null;
		entt::entity defender = entt::null;
		entt::entity weapon = entt::null;
		entt::entity ammunition = entt::null;
		int advantage = 0;
	};
	struct AttackHit
	{
		entt::entity attacker = entt::null;
		entt::entity defender = entt::null;
		entt::entity weapon = entt::null;
		entt::entity ammo = entt::null;
		int advantage = 0;
	};
	struct AttackMiss
	{
		entt::entity attacker = entt::null;
		entt::entity defender = entt::null;
		entt::entity weapon = entt::null;
		entt::entity ammo = entt::null;
		int advantage = 0;
	};
	struct TakeDamage
	{
		entt::entity target = entt::null;
		entt::entity source = entt::null;
		Enum::DamageType damage_type = Enum::DamageType::None;
		size_t amount = 0;
	};
	struct Spawn
	{
		entt::entity entity = entt::null;
		Game::World::GlobalPosition position{};
	};
	struct Destroy
	{
		entt::entity entity = entt::null;
	};
	struct Drop
	{
		entt::entity entity = entt::null;
		entt::entity item = entt::null;
		Game::World::GlobalPosition position{};
	};
	struct Take
	{
		entt::entity entity = entt::null;
		entt::entity item = entt::null;
		Game::World::GlobalPosition position{};
	};
	struct Equip
	{
		entt::entity entity = entt::null;
		entt::entity equipment = entt::null;
	};
	struct Unequip
	{
		entt::entity entity = entt::null;
		entt::entity equipment = entt::null;
	};
	struct DiceRoll
	{
		Dice dice{};
		int result = 0;
		int difficulty = 0;
	};
	struct Death
	{
		entt::entity entity = entt::null;
	};
	struct BecomeHostile
	{
		entt::entity entity = entt::null;
		entt::entity target = entt::null;
	};
	struct ReceiveItem
	{
		entt::entity entity = entt::null;
		entt::entity item = entt::null;
	};
	using Any = std::variant<
		Null,
		Move,
		LeavePosition,
		EnterPosition,
		Bump,
		Attack,
		AttackHit,
		AttackMiss,
		TakeDamage,
		Spawn,
		Destroy,
		Drop,
		Take,
		Equip,
		Unequip,
		DiceRoll,
		Death,
		BecomeHostile,
		ReceiveItem,
		LoseMovementPoints
			>;
	using List = std::vector<Any>;
	enum class Outcome // was it accepted into the game simulation?
	{
		Accepted,
		Rejected
	};
	struct Result
	{
		Outcome outcome;
		List consequences;

		static inline Result rejected()
		{
			return Result{
				.outcome = Outcome::Rejected,
					.consequences = {}
			};
		}
	};

	struct Node
	{
		Any event;
		std::optional<Outcome> outcome = std::nullopt;
		std::vector<Node> consequences;
	};
} // namespace Game::Event
