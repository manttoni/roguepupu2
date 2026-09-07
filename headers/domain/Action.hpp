#pragma once

#include <variant>

#include "domain/Position.hpp"
#include "external/entt/entt.hpp"

namespace Domain::Action
{

	struct Null
	{
	};
	struct Move
	{
		entt::entity entity = entt::null;
		Domain::Position destination{};
	};
	struct Attack
	{
		entt::entity attacker = entt::null;
		entt::entity defender = entt::null;
	};
	struct EndTurn
	{
		entt::entity entity = entt::null;
	};
	struct Equip
	{
		entt::entity entity = entt::null;
		entt::entity item = entt::null;
	};
	struct Unequip
	{
		entt::entity entity = entt::null;
		entt::entity item = entt::null;
	};
	struct UseAbility
	{
		entt::entity entity = entt::null;
		entt::entity target = entt::null;
		entt::entity ability = entt::null;
	};
	struct CastSpell
	{
		entt::entity entity = entt::null;
		entt::entity target = entt::null;
		entt::entity spell = entt::null;
	};
	struct QuitGame
	{
	};

	using Any = std::variant<
		Null,
		Move,
		Attack,
		EndTurn,
		Equip,
		Unequip,
		UseAbility,
		CastSpell,
		QuitGame
			>;

}
