#pragma once

#include <cstddef>
#include <variant>
#include "domain/Position.hpp"
#include "external/entt/entt.hpp"
#include "domain/Enum.hpp"
#include "domain/Dice.hpp"

struct NullEvent
{
};
struct MoveEvent
{
	entt::entity entity = entt::null;
	Position from{};
	Position to{};
};
struct AttackEvent
{
	entt::entity attacker = entt::null;
	entt::entity defender = entt::null;
	entt::entity weapon = entt::null;
	entt::entity ammunition = entt::null;
	int advantage = 0;
};
struct AttackHitEvent
{
	entt::entity attacker = entt::null;
	entt::entity defender = entt::null;
	entt::entity weapon = entt::null;
	entt::entity ammo = entt::null;
	int advantage = 0;
};
struct AttackMissEvent
{
	entt::entity attacker = entt::null;
	entt::entity defender = entt::null;
	entt::entity weapon = entt::null;
	entt::entity ammo = entt::null;
	int advantage = 0;
};
struct TakeDamageEvent
{
	entt::entity target = entt::null;
	entt::entity source = entt::null;
	Enum::DamageType damage_type = Enum::DamageType::None;
	size_t amount = 0;
};
struct SpawnEvent
{
	entt::entity entity = entt::null;
	Position position{};
};
struct DestroyEvent
{
	entt::entity entity = entt::null;
};
struct DropEvent
{
	entt::entity entity = entt::null;
	entt::entity item = entt::null;
	Position position{};
};
struct TakeEvent
{
	entt::entity entity = entt::null;
	entt::entity item = entt::null;
	Position position{};
};
struct EquipEvent
{
	entt::entity entity = entt::null;
	entt::entity equipment = entt::null;
};
struct UnequipEvent
{
	entt::entity entity = entt::null;
	entt::entity equipment = entt::null;
};
struct DiceRollEvent
{
	Dice dice{};
	int result = 0;
	int difficulty = 0;
};
struct DeathEvent
{
	entt::entity entity = entt::null;
};
struct BecomeHostileEvent
{
	entt::entity entity = entt::null;
	entt::entity target = entt::null;
};

using Event = std::variant<
	NullEvent,
	MoveEvent,
	AttackEvent,
	AttackHitEvent,
	AttackMissEvent,
	TakeDamageEvent,
	SpawnEvent,
	DestroyEvent,
	DropEvent,
	TakeEvent,
	EquipEvent,
	UnequipEvent,
	DiceRollEvent,
	DeathEvent,
	BecomeHostileEvent
>;
