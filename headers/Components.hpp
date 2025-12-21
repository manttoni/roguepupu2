#pragma once

#include <string>     // for basic_string, string
#include "Color.hpp"  // for Color
#include "entt.hpp"   // for null, null_t, entity, vector
class Cell;

struct Solid {};
struct Opaque {};
struct Player {};
struct TwoHanded {};

struct Name { std::string name; };
struct Position { Cell* cell; };
struct Glyph { wchar_t glyph; };

struct FGColor { Color color; };
struct BGColor { Color color; };

struct Vision { int range; };
struct Weight { double weight; };
struct Faction { std::string faction; };

struct Category { std::string category; };
struct Subcategory { std::string subcategory; };

struct ArmorPenetration { int armor_penetration; };
struct Armor { int armor; };

struct Accuracy { int accuracy; };
struct Evasion { int evasion; };

struct Barrier { int barrier; };
struct Power { int power; };

struct CritChance { double crit_chance; };
struct CritMultiplier { double crit_multiplier; };

struct Glow { double strength, radius; };
struct Damage
{
	int min, max;
	std::string type;
};

struct Level { int level; };
struct Attributes
{
	int strength;
	int dexterity;
	int intelligence;
};
struct Resources
{
	int health, fatigue, mana;
};
struct Actions
{
	int total, used;
};

struct Equipment
{
	entt::entity right_hand{entt::null};
	entt::entity left_hand{entt::null};
	entt::entity armor{entt::null};
};
struct Inventory
{
	std::vector<entt::entity> inventory;
};
struct Portal
{
	entt::entity destination;
};
