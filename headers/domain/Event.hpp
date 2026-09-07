#pragma once

#include <cstddef>
#include <variant>
#include "domain/Position.hpp"
#include "external/entt/entt.hpp"
#include "domain/Enum.hpp"
#include "domain/Dice.hpp"

namespace Domain::Event
{
	struct Null
	{
	};
	struct Move
	{
		entt::entity entity = entt::null;
		Domain::Position from{};
		Domain::Position to{};
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
		Domain::Position position{};
	};
	struct Destroy
	{
		entt::entity entity = entt::null;
	};
	struct Drop
	{
		entt::entity entity = entt::null;
		entt::entity item = entt::null;
		Domain::Position position{};
	};
	struct Take
	{
		entt::entity entity = entt::null;
		entt::entity item = entt::null;
		Domain::Position position{};
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
		ReceiveItem
			>;
} // namespace Domain::Event
