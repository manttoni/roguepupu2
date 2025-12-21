#pragma once

#include "entt.hpp"
class Cell;
namespace ActionSystem
{

	enum class Type
	{
		None,
		ExamineCell,
		OpenInventory,
		ShowPlayer,
		Move,
		Attack,
		Transition,
		Unlock,
		Open,
		DoNothing,
	};
	struct Intent
	{
		Type type = Type::None;
		Cell* target_cell = nullptr;
		entt::entity target = entt::null;
	};
	void player_turn(entt::registry& registry);
	void npc_turn(entt::registry& registry);
	bool can_act(const entt::registry& registry, const entt::entity actor);
	void use_action(entt::registry& registry, const entt::entity actor);
};
