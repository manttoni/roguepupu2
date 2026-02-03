#pragma once

#include <regex>
#include <ncurses.h>
#include <cmath>
#include <string>
#include <map>
#include <vector>
#include <optional>
#include "utils/Error.hpp"
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
struct NcursesAttr
{
	chtype attr;

	std::string markup() const
	{
		switch (attr)
		{
			case A_DIM:
				return "[A_DIM]";
			case A_BOLD:
				return "[A_BOLD]";
			case A_REVERSE:
				return "[A_REVERSE]";
			default:
				return "[A_NORMAL]";
		}
	}

	inline static bool is_markup(const std::string& str, const size_t idx)
	{
		if (idx >= str.size() || str[idx] != '[')
			return false;

		const auto close = str.find(']', idx);
		if (close == std::string::npos)
			return false;

		const auto markup = str.substr(idx, close - idx + 1);
		if (markup == "[reset]")
			return true;
		std::regex regex(R"(\[(A_[A-Z_]*)\])");
		std::smatch match;
		if (!std::regex_match(markup, match, regex))
			return false;

		const std::vector<std::string> handled_attrs = {"A_BOLD", "A_DIM", "A_REVERSE", "A_NORMAL"};
		auto it = std::find(handled_attrs.begin(), handled_attrs.end(), match[1]);
		return it != handled_attrs.end();
	}

	inline static chtype from_markup(const std::string& str, const size_t idx)
	{
		assert(is_markup(str, idx));
		const auto close = str.find(']', idx);
		const auto markup = str.substr(idx + 1, close - idx - 1);
		if (markup == "A_DIM") return A_DIM;
		if (markup == "A_BOLD") return A_BOLD;
		if (markup == "A_REVERSE") return A_REVERSE;
		return A_NORMAL;
	}

};

/* State */
struct Vision { double range; };
struct Hidden {};
struct Invisible {};
struct Experience { size_t amount; };	// Derived: level, if there will be a level
struct Dead { size_t turn_number; };	// turn_number might be unused

struct Vitality { int value; };			// Derived: max_health
struct Endurance { int value; };		// Derived: max_stamina
struct Willpower { int value; };		// Derived: max_mana
struct Perception { int value; };		// Derived: cell opaqueness. Higher perception -> better vision
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
	enum class Type
	{
		LawfulGood,
		LawfulNeutral,
		LawfulEvil,
		NeutralGood,
		TrueNeutral,
		NeutralEvil,
		ChaoticGood,
		ChaoticNeutral,
		ChaoticEvil
	};
	double chaos_law = 0.0;
	double evil_good = 0.0;
	double tolerance = 0.0;

	Alignment() = default;
	Alignment(const double chaos_law, const double evil_good, const double tolerance = 0.0)
		: chaos_law(chaos_law), evil_good(evil_good), tolerance(tolerance) {}
	Alignment(const Type type, const double tolerance = 0.0) : tolerance(tolerance)
	{
		switch (type)
		{
			case Type::LawfulGood: chaos_law = 1; evil_good = 1; break;
			case Type::LawfulNeutral: chaos_law = 1; evil_good = 0; break;
			case Type::LawfulEvil: chaos_law = 1; evil_good = -1; break;
			case Type::NeutralGood: chaos_law = 0; evil_good = 1; break;
			case Type::TrueNeutral: chaos_law = 0; evil_good = 0; break;
			case Type::NeutralEvil: chaos_law = 0; evil_good = -1; break;
			case Type::ChaoticGood: chaos_law = -1; evil_good = 1; break;
			case Type::ChaoticNeutral: chaos_law = -1; evil_good = 0; break;
			case Type::ChaoticEvil: chaos_law = -1; evil_good = -1; break;
		}
	}

	std::string to_string() const
	{
		std::string ret = "";
		if (chaos_law <= -0.5) ret = "Chaotic ";
		else if (chaos_law < 0.5) ret = "Neutral ";
		else ret = "Lawful ";

		if (evil_good <= -0.5) ret += "Evil";
		else if (evil_good < 0.5) ret += "Neutral";
		else ret += "Good";

		if (ret == "Neutral Neutral") return "True Neutral";
		return ret;
	}

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
	};

	Slot slot;
	static Slot from_string(const std::string& str)
	{
		if (str == "one_handed") return Slot::OneHanded;
		if (str == "two_handed") return Slot::TwoHanded;
		Error::fatal("Invalid Equipment::Slot string");
	}
	std::string to_string() const
	{
		switch (slot)
		{
			case Slot::OneHanded: return "one handed";
			case Slot::TwoHanded: return "two handed";
			default: Error::fatal("Equipment::Slot to_string error");
		}
	}
};
struct Tool
{
	enum class Type
	{
		None,
		Felling,
		Cutting,
		Mining,
	};
	Type type = Type::None;
	static Type from_string(const std::string& str)
	{
		if (str == "none") return Type::None;
		if (str == "felling") return Type::Felling;
		if (str == "cutting") return Type::Cutting;
		if (str == "mining") return Type::Mining;
		Error::fatal("Invalid Tool::Type string");
	}
	std::string to_string() const
	{
		switch (type)
		{
			case Type::None: return "none";
			case Type::Felling: return "felling";
			case Type::Cutting: return "cutting";
			case Type::Mining: return "mining";
			default: Error::fatal("Type convert error");
		}
	}

};
struct Throwable {};
struct Range
{
	double cells; // 1 is up/down/left/right, 1.5 is also diagonal etc
};
struct Edge { double sharpness; };

/* Crafting related */
struct Gatherable
{
	enum class Effect
	{
		None,
		Dim,
		Destroy,
	};

	static Effect from_string(const std::string& str)
	{
		if (str == "none") return Effect::None;
		if (str == "dim") return Effect::Dim;
		if (str == "destroy") return Effect::Destroy;
		Error::fatal("Invalid Gatherable::Effect string");
	}

	Effect effect = Effect::None; // what happens when gathered
	Tool::Type tool_type;
	std::vector<std::string> loot_table_ids;
};

/* Liquid */
struct LiquidContainer
{
	double capacity = 0.0;
};

/* This is probably going to change, but exist
 * Other names include "Location"
 * "Encounters" have "Locations"
 * This can also be a "Territory" by some animal
 * */
struct BaseLocation
{
	size_t idx;
	double radius;
};

/* Light */
struct Glow
{
	double intensity;
	double radius;
};

/* This can be expanded. Add for example enum class to
 * describe the destination in case it has to be generated.
 * */
struct Transition
{
	entt::entity destination = entt::null;

	bool operator==(const Transition& other) const = default;
};
struct DestroyWhenStacked {}; // Maybe just a relic, but spider webs have to be destroyed when you walk on them

/* Other */
struct AI
{
	bool aggressive = true; // attack on sight if has bad opinion of target
							// is it gonna be just a bunch of booleans? Gatherable was going to be but didnt end up like that
};
struct Abilities
{
	std::map<std::string, Ability> abilities;
};

struct Player {};
struct Destroyed {};


