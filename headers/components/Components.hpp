#pragma once

#include <string>
#include <map>
#include <vector>
#include <optional>
#include "utils/Log.hpp"
#include "domain/Color.hpp"
#include "domain/Intent.hpp"
#include "domain/Ability.hpp"
#include "domain/Trigger.hpp"
#include "external/entt/fwd.hpp"

/* Core components */
struct Name { std::string name; }; // id
struct Category { std::string category; }; // data/entities/category/subcategory.json
struct Subcategory { std::string subcategory; };

/* Optional components... */
/* Physical */
struct Solid {};
struct Opaque {};
struct Landmark {}; // probably unused
struct Size { double size = 0.0; };
struct Weight { double weight; };

/* Rendering */
struct Glyph { wchar_t glyph; };
struct FGColor { Color color; };
struct BGColor { Color color; };

/* State */
struct Vision { double range; };
struct Hidden {};
struct Invisible {};
struct Experience { size_t amount; }	// Derived: level
struct Dead { size_t turn_number; };

// combat system, needs more planning
struct Strength { int value; }			// Derived: damage with blunt, stamina loss
struct Dexterity { int value; }			// Derived: damage with sharp, stamina loss
struct Intelligence { int value; }		// Derived: ?
struct Vitality { int value; }			// Derived: max_health
struct Endurance { int value; }			// Derived: max_stamina
struct Willpower { int value; }			// Derived: max_mana

struct Health { int current; }
struct Stamina { int current; }
struct Mana { int current; }

struct EquipmentSlots
{
	std::map<Equipment::Slot, std::optional<entt::entity>> slots;
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
	Tool::Type tool_type;
	std::string entity_id;
	size_t amount;
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

/* Other */
struct Transition
{
	entt::entity destination = entt::null;
};
struct AI // Intent structs in order of priority
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




