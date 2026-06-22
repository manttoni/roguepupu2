#pragma once

#include <limits>
#include <unordered_map>
#include "domain/Color.hpp"
#include "domain/Dice.hpp"
#include "utils/Range.hpp"
#include "utils/Error.hpp"
#include "utils/Random.hpp"

namespace Damage
{
	enum class Type
	{
		None,
		Piercing,
		Slashing,
		Bludgeoning,
		Burning,
		Bleeding,
		Poison,
	};

	std::string type_to_string(const Type type);
	Type string_to_type(const std::string& str);
	Color get_color(const Type type);

	struct Roll
	{
		Type type = Type::None;
		Dice dice;
		int modifier = 0;
		int result = 0;
		bool is_rolled = false;
		int advantage = 0;

		Roll() = default;
		Roll(const Type type, const Dice dice, const int modifier = 0, const int advantage = 0) : type(type), dice(dice), modifier(modifier), advantage(advantage) {}
		Roll(const Type type, const int modifier) : type(type), modifier(modifier) {}

		void roll(const int advantage = 0);

		bool operator==(const Roll& other) const = default;
		bool operator!=(const Roll& other) const = default;

	};
};

inline std::ostream& operator<<(std::ostream& os, const Damage::Type& type)
{
	os << Damage::get_color(type).markup();
	switch (type)
	{
		case Damage::Type::None:		os << "none"; break;
		case Damage::Type::Piercing:	os << "piercing"; break;
		case Damage::Type::Slashing:	os << "slashing"; break;
		case Damage::Type::Bludgeoning:	os << "bludgeoning"; break;
		case Damage::Type::Burning:		os << "burning"; break;
		case Damage::Type::Bleeding:	os << "bleeding"; break;
		case Damage::Type::Poison:		os << "poison"; break;
		default: Error::fatal("Unhandled damage type");
	}
	return os << "{reset}";
}

inline std::ostream& operator<<(std::ostream& os, const Damage::Roll& roll)
{
	if (roll.is_rolled)
		os << roll.result << " ";

	os << roll.type << " damage";
	os << " (" << roll.dice << (roll.modifier > 0 ? "+" + std::to_string(roll.modifier) : (roll.modifier < 0 ? std::to_string(roll.modifier) : "")) << ")";

	return os;
}

