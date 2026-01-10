#pragma once

#include <string>     // for basic_string, string
#include "Color.hpp"  // for Color
#include "entt.hpp"   // for null, null_t, entity, vector
#include "Event.hpp"
#include "Utils.hpp"
#include "LiquidMixture.hpp"
class Cell;

struct Solid {};
struct Opaque {};
struct Player {};
struct TwoHanded {};
struct Landmark {};
struct Hidden {};

struct Name { std::string name; };
struct Position { Cell* cell; };
struct Glyph { wchar_t glyph; };

struct FGColor { Color color; };
struct BGColor { Color color; };

struct Vision { double range; };
struct Weight { double weight; };
struct Faction { std::string faction; };

struct Category { std::string category; };
struct Subcategory { std::string subcategory; };

struct Glow { double strength, radius; };
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
struct Equipment
{
	enum class Slot
	{
		OneHanded,
		TwoHanded,
		Begin,
		LeftHand,
		RightHand,
		Count,
	};
	static Slot from_string(const std::string& str)
	{
		static const std::map<std::string, Slot> map =
		{
			{"one_handed", Slot::OneHanded},
			{"two_handed", Slot::TwoHanded},
			{"left_hand", Slot::LeftHand},
			{"right_hand", Slot::RightHand}
		};
		auto it = map.find(str);
		if (it == map.end())
			Log::error("Unknown equipment slot: " + str);
		return it->second;
	}
	Slot slot;
};
struct EquipmentSlots
{
	std::map<Equipment::Slot, std::optional<entt::entity>> slots;
};
struct Inventory
{
	std::vector<entt::entity> inventory;
};
struct Transition
{
	entt::entity destination;
};
struct AI
{
	std::vector<Intent> intentions;
};
struct Abilities
{
	std::map<std::string, Ability> abilities;
};
struct Triggers
{
	std::vector<Trigger> triggers;
};

struct EventQueue
{
	std::vector<Event> queue;
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

struct Gatherable
{
	Tool::Type tool_type;
	std::string entity_id;
	size_t amount;
};

struct Dead
{
	size_t turn_number;
};

struct BaseLocation
{
	size_t idx;
	double radius;
};

struct Edge
{
	double edge;
};

struct Throwable {};

struct LiquidSource
{
	Liquid::Type type = Liquid::Type::None;
	double rate = 0.0;
	double volume_left = std::numeric_limits<double>::infinity();
};

struct Size
{
	double size = 0.0;
};
