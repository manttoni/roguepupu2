#pragma once

#include <optional>
#include <string>
#include "Cell.hpp"
#include "Color.hpp"
#include "Dice.hpp"
#include "systems/EquipmentSystem.hpp"

struct Name
{
	std::string name;
};

struct Position
{
	Cell* cell;
};

struct Renderable
{
	wchar_t glyph;
	Color color;
};

struct Glow
{
	double strength;
	double radius;
};

struct Vision
{
	int range;
};

struct Inventory
{
	std::vector<entt::entity> inventory;
};

struct Solid {};
struct Opaque {};
struct Player {};

struct Damage
{
	Dice dice;
};

struct Weight
{
	double weight;
};

struct Equippable
{
	EquipmentSystem::Slot slot;
};

struct Equipment
{
	std::map<EquipmentSystem::Slot, std::optional<entt::entity>> slots =
	{
		{ EquipmentSystem::Slot::left_hand, std::nullopt },
		{ EquipmentSystem::Slot::right_hand, std::nullopt }
	};
};
