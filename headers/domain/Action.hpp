#pragma once

#include <variant>

#include "domain/Position.hpp"
#include "external/entt/entt.hpp"

struct NullAction
{
};
struct MoveAction
{
	entt::entity entity = entt::null;
	Position destination{};
};
struct AttackAction
{
	entt::entity attacker = entt::null;
	entt::entity defender = entt::null;
};
struct EndTurnAction
{
	entt::entity entity = entt::null;
};
struct EquipAction
{
	entt::entity entity = entt::null;
	entt::entity item = entt::null;
};
struct UnequipAction
{
	entt::entity entity = entt::null;
	entt::entity item = entt::null;
};
struct UseAbilityAction
{
	entt::entity entity = entt::null;
	entt::entity target = entt::null;
	entt::entity ability = entt::null;
};
struct CastSpellAction
{
	entt::entity entity = entt::null;
	entt::entity target = entt::null;
	entt::entity spell = entt::null;
};
struct EscapeAction
{
}; // Exit game

using Action = std::variant<
NullAction,
	MoveAction,
	AttackAction,
	EndTurnAction,
	EquipAction,
	UnequipAction,
	UseAbilityAction,
	CastSpellAction,
	EscapeAction
	>;
