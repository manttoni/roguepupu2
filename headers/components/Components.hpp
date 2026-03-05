#pragma once

#include <cmath>
#include <map>
#include <ncurses.h>
#include <optional>
#include <regex>
#include <string>
#include <vector>
#include "utils/Error.hpp"
#include "utils/Range.hpp"
#include "domain/Color.hpp"
#include "domain/Intent.hpp"
#include "domain/Ability.hpp"
#include "external/entt/fwd.hpp"

/* Core components */
struct Name { std::string name; };
struct Glyph { wchar_t glyph; };
// Color is in domain, a class
struct Solid { bool value = true; };
struct Opaque { double value = 1.0; };	// value [0,1] is how much this entity blocks vision. 1 = completely opaque, 0, transparent
struct Mass { double value; };

// move this somewhere, is similar to Color
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
			case A_BLINK:
				return "[A_BLINK]";
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

		const std::vector<std::string> handled_attrs = {"A_BOLD", "A_DIM", "A_REVERSE", "A_NORMAL", "A_BLINK"};
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
		if (markup == "A_BLINK") return A_BLINK;
		return A_NORMAL;
	}

	bool operator==(const NcursesAttr& other) const = default;
	bool operator==(const chtype attr) const { return attr == this->attr; }
	bool operator!=(const NcursesAttr& other) const = default;
	bool operator!=(const chtype attr) const { return attr != this->attr; }
};

/* State */
struct Hidden {};
struct Invisible {};
struct Experience { size_t amount; };
struct Dead { size_t turn_number; };

struct Vitality { int value; };		// health
struct Endurance { int value; };	// stamina
struct Willpower { int value; };	// mana
struct Charisma { int value; };		// opinion
struct Perception { int value; };	// vision range
struct Strength { int value; };
struct Agility { int value; };
struct Dexterity { int value; };

template<typename T> struct Buff
{
	int value;
	std::optional<size_t> duration;

	bool operator==(const Buff& other) const = default;
};
template<typename T> struct BuffContainer
{
	std::vector<Buff<T>> buffs;
};

struct Health { int current; };
struct Stamina { int current; };
struct Mana { int current; };

// This feels like it should have its own file
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
	std::unordered_map<entt::entity, double> personal_opinions;

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
struct EquipmentSlot
{
	enum class Slot
	{
		MainHand, // weapon or shield
		OffHand, // weapon or shield
		Ammo, // this is used when firing ranged weapons
				 // rest are armor or things like that
	};
	std::optional<std::vector<Slot>> use_all; // this equipment uses all of these slots (f.e. two handed weapons)
	std::optional<std::vector<Slot>> use_one; // this equipment uses one of these slots (f.e. one handed weapons)
};
struct EquipmentSlots
{
	using Slot = EquipmentSlot::Slot;

	struct Loadout
	{	// These are for players quality of life, just press w
		entt::entity main_hand = entt::null;
		entt::entity off_hand = entt::null;
		entt::entity ammo = entt::null;
	};
	std::array<Loadout, 2> loadouts;
	size_t active_loadout = 0;

	const Loadout& get_active_loadout() const
	{
		return loadouts[active_loadout];
	}
	const Loadout& get_other_loadout() const
	{
		return loadouts[active_loadout ^ 1];
	}

	std::map<Slot, entt::entity> equipped_items{
		{ Slot::MainHand, entt::null },
		{ Slot::OffHand, entt::null },
		{ Slot::Ammo, entt::null }
	};
};
struct Inventory
{
	std::vector<entt::entity> items;
};

/* Weapons etc... */
struct AttackRange
{
	Range<double> range;
};

/* Crafting related */


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
	Position position;
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

/* Other */
struct AI
{
	bool idle_wander = false;
	bool aggressive = false;
};
struct Abilities
{
	std::map<std::string, Ability> abilities; // this is actually not implemented at all (anymore)
};
struct LootTableRef { std::string id; };

/* Booleans */
struct Closed { bool value; };
struct Stackable { bool value; }; // This needs some number to tell how much is stacked
struct DestroyWhenStacked { bool value; }; // TODO: rename, does not mean stacked in inventory, but in cell, aka stepped on/fragile

/* Enums and their aliases. Might need struct wrapping, but these at least dont look like they will conflict */
enum class AmmoType { None, Arrow, Bolt, Bullet };
enum class ToolType { None, Cutting, Felling, Mining };
using RequiresTool = ToolType;
using RequiresAmmo = AmmoType;
enum class GatherEffect { None, Dim, Destroy };

/* Tags - contain no data, but are data themselves by existing
 * Check data/components/tag_dependencies.json
 * A Tag comes with Components which have the necessary data
 * A Tag can also come with other Tags, f.e. all Weapons are also Items.
 * A Tag is a promise that the entity will fulfill that functionality
 * */
struct Creature {}; // is something like an animal
struct Player {}; // player can control this
struct NPC {}; // AI controls this
struct Destroyed {}; // Mark entity for destruction and destroy it at a certain point
struct Equipment {}; // This entity is a piece of equipment
struct Door {}; // is a door
struct Item {}; // is an item
struct Tool {}; // is a tool
struct Weapon {}; // is a weapon
struct FinesseWeapon {}; // used with dexterity
struct VersatileWeapon {}; // if off_hand is empty, is more effective
struct MechanicalWeapon {}; // is a machine, usual attributes dont affect usage
struct MeleeWeapon {}; // attack adjacent cells, also diagonal
struct ImprovisedWeapon {}; // not supposed to be used as a melee weapon, but its possible
struct ThrowingWeapon {}; // is good for throwing
struct RangedWeapon {}; // can attack distant targets, uses ammo
struct Ammo{}; // is ammo
struct Gatherable {}; // can be gathered with tools
struct Mushroom {}; // is a mushroom growing naturally
struct Plant {}; // is a plant growing naturally
