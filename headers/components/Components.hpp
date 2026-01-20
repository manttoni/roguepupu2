#pragma once

#include <cmath>
#include <string>
#include <map>
#include <vector>
#include <optional>
#include "utils/Log.hpp"
#include "domain/Color.hpp"
#include "domain/Intent.hpp"
#include "domain/Ability.hpp"
#include "external/entt/fwd.hpp"

/* Core components */
struct Name
{
	std::string name;

	bool operator==(const Name& other) const = default;
};
struct Category
{
	std::string category;

	bool operator==(const Category& other) const = default;
}; // data/entities/category/subcategory.json
struct Subcategory
{
	std::string subcategory;

	bool operator==(const Subcategory& other) const = default;
};

/* Optional components... */
/* Physical */
struct Solid
{
	bool value = true;

	bool operator==(const Solid& other) const = default;
};
struct Opaque { double value = 1.0; };	// value [0,1] is how much this entity blocks vision. 1 = completely opaque, 0, transparent
struct Size { double liters; };
struct Weight { double kilograms; };

/* Rendering */
struct Glyph { wchar_t glyph; };
struct FGColor { Color color; };
struct BGColor { Color color; };

/* State */
struct Vision { double range; };
struct Hidden {};
struct Invisible {};
struct Experience { size_t amount; };	// Derived: level
struct Dead { size_t turn_number; };

struct Vitality { int value; };			// Derived: max_health
struct Endurance { int value; };		// Derived: max_stamina
struct Willpower { int value; };		// Derived: max_mana
struct Perception { int value; };		// Derived: vision_range
struct Charisma { int value; };			// Affects opinion
template<typename T> struct Buff
{
	int value;
	std::optional<size_t> duration;
};
template<typename T> struct BuffContainer
{
	std::vector<Buff<T>> buffs;
};

struct Health { int current; };
struct Stamina { int current; };
struct Mana { int current; };
struct Alignment
{
	double tolerance = 0.0;
	double chaos_law = 0.0;
	double evil_good = 0.0;

	double distance(const Alignment& other) const
	{
		return hypot(chaos_law - other.chaos_law, evil_good - other.evil_good);
	}
};
/* Inventory, gear, equipment... */
struct EquipmentSlots
{
	std::vector<entt::entity> equipped_items;
};
struct Inventory
{
	std::vector<entt::entity> inventory;
};

/* Equipment, weapons etc... */
struct Equipment
{
	enum class Slot
	{
		OneHanded,
		TwoHanded,
		Count,
	};
	static Slot from_string(const std::string& str)
	{
		static const std::map<std::string, Slot> map =
		{
			{"one_handed", Slot::OneHanded},
			{"two_handed", Slot::TwoHanded},
		};
		auto it = map.find(str);
		if (it == map.end())
			Log::error("Unknown equipment slot: " + str);
		return it->second;
	}
	Slot slot;
};
struct Tool
{
	enum class Type
	{
		None,
		Felling,
		Cutting,
		Mining
	};
	static Type from_string(const std::string& str)
	{
		static const std::map<std::string, Type> map =
		{
			{"none", Type::None},
			{"felling", Type::Felling},
			{"cutting", Type::Cutting},
			{"mining", Type::Mining}
		};
		auto it = map.find(str);
		if (it == map.end())
			Log::error("Unknown tool type: " + str);
		return it->second;
	}
	Type type;
};
struct Edge { double sharpness; };	// Derived: damage
struct Throwable { double range = 0.0; };

/* Crafting related */
struct Gatherable
{
	bool destroy = false;
	bool lose_glow = false;
	Tool::Type tool_type;
	std::vector<std::string> loot_table_ids;
};

/* Liquid */
struct LiquidContainer
{
	double capacity = 0.0;
};

/* Faction, bases, home */
struct Faction { std::string faction; }; // needs planning
struct BaseLocation // spawnpoint
{
	size_t idx;
	double radius;
};

/* Light */
struct Glow { double strength, radius; };

/* Reactive */
struct Transition
{
	entt::entity destination = entt::null;

	bool operator==(const Transition& other) const = default;
};
struct DestroyWhenStacked {};

/* Other */
struct AI
{
	bool aggressive = true;
};
struct Abilities
{
	std::map<std::string, Ability> abilities;
};

struct Player {};
struct Destroyed {};


