#pragma once

#include <optional>
#include <string>
#include "Cell.hpp"
#include "Color.hpp"
#include "Dice.hpp"
#include "systems/EquipmentSystem.hpp"
#include "systems/DamageSystem.hpp"

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
	DamageSystem::Type type;
	Dice dice;
};

struct Weight
{
	double lb;
};

struct Equipment
{
	entt::entity right_hand;
	entt::entity left_hand;
	entt::entity armor;
};

struct Category
{
	std::string category;
};

struct Subcategory
{
	std::string subcategory;
};

struct Rarity
{
	std::string rarity;
};

struct Value
{
	size_t value;
};

struct Weapon
{
	std::string proficiency;
	std::vector<std::string> properties;
};

struct Armor
{
	std::string proficiency;
	size_t armor_class;
	size_t max_dexbonus;
	bool stealth_disadvantage;
	size_t strength_requirement;
};
