#pragma once

#include <optional>
#include <string>
#include "Cell.hpp"
#include "Color.hpp"
#include "Dice.hpp"
#include "systems/EquipmentSystem.hpp"
#include "systems/DamageSystem.hpp"
#include "entt.hpp"

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
	std::string type;
	Dice dice;
};

struct Weight
{
	double lb;
};

struct Equipment
{
	entt::entity right_hand{entt::null};
	entt::entity left_hand{entt::null};
	entt::entity armor{entt::null};
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
	double normal_range; // distance in cells
	double long_range; // disadvantage outside normal range
	Dice versatile_dice;
};

struct Armor
{
	std::string proficiency;
	size_t armor_class;
	size_t max_dexbonus;
	std::string stealth;
	size_t strength_requirement;
};

struct Stats
{
	size_t level;
	std::map<std::string, size_t> attributes;
	size_t max_hp;
	size_t hp;
};

