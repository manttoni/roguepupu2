#pragma once

#include "domain/Color.hpp"
#include "utils/Error.hpp"

/* This is a way to describe type and amount of damage
 *
 * Piercing: stabbing with sharp weapons, sharp projectiles
 * Slashing: slashing with sharp weapons
 * Bludgeoning: blunt weapons and projectiles, most unarmed attacks, striking with hilt/pommel
 *
 * Burning: burning status effect
 * Bleeding: bleeding status effect
 * Poison: poison status effect
 * */

struct Damage
{
	enum class Type
	{
		Piercing,
		Slashing,
		Bludgeoning,
		Burning,
		Bleeding,
		Poison,
	};

	Type type;
	size_t amount;

	Damage() = delete;
	Damage(const Type type, const size_t amount) : type(type), amount(amount) {}

	std::string type_string() const
	{
		switch (type)
		{
			case Type::Piercing: return "piercing";
			case Type::Slashing: return "slashing";
			case Type::Bludgeoning: return "bludgeoning";
			case Type::Burning: return "burning";
			case Type::Bleeding: return "bleeding";
			case Type::Poison: return "poison";
			default: Error::fatal("Unhandled damage type");
		}
	}

	Color get_color() const
	{
		switch (type)
		{
			case Type::Piercing:
			case Type::Slashing:
			case Type::Bludgeoning:
				return Color(500, 500, 500);
			case Type::Burning:
				return Color(400, 100, 200);
			case Type::Bleeding:
				return Color(400, 0, 0);
			case Type::Poison:
				return Color(100, 400, 100);
			default: Error::fatal("Unhandled damage type");
		}
	}

	std::string to_string() const
	{
		return get_color().markup() + std::to_string(amount) + " " + type_string() + "{reset}";
	}
};
