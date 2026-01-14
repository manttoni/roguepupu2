#pragma once

#include <optional>
#include "entt.hpp"
#include "Color.hpp"
#include "Components.hpp"

class Cell;

struct Target
{
	enum class Type
	{
		None,
		Self,
		Actor,
		Cell,
		Entity,
		Enemy,
		Ally,
	};

	Type type = Type::None;
	entt::entity entity = entt::null;
	Position position{};
	double range = 0.0;
};

struct Effect
{
	enum class Type
	{
		None,
		CreateEntity,
		DestroyEntity,
		Transition,
		SetFGColor,
	};

	Type type = Type::None;
	std::string entity_id;
	Color fgcolor;
};


struct Ability
{
	enum class Type
	{
		None,
		Spell,
		Innate,
	};

	Type type = Type::None;
	std::string id = "";
	size_t cooldown = 0;
	size_t last_used = 0;
	Effect effect{};
	Target target{};
};
struct Conditions
{
	std::optional<double> weight_min, weight_max;
	std::optional<std::string> category, category_not;
	std::optional<double> hp_min, hp_max;
};

struct Intent
{
	enum class Type
	{
		None,
		ExamineCell,
		OpenInventory,
		ShowPlayer,
		Move,
		Attack,
		Unlock,
		Open,
		DoNothing,
		UseAbility,
		Hide,
		Flee,
		Gather,
	};

	Type type = Type::None;
	Target target{};
	std::string ability_id = "";
	Conditions conditions{};
	entt::entity actor = entt::null;
};

struct Trigger
{
	enum class Type
	{
		None,
		EnterCell,
		Gather,
	};

	Type type = Type::None;
	Effect effect;
	Target target;
	Conditions conditions;
};

struct Event
{
	enum class Type
	{
		None,
		Move,
		Gather,
	};

	Type type = Type::None;
	entt::entity actor = entt::null;
	entt::entity gatherable = entt::null;
	Position move_from{};
	Position move_to{};
};
