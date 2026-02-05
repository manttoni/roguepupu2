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
	Damage(const std::string& str, const size_t amount) : amount(amount)
	{
		if (str == "piercing") type = Type::Piercing;
		else if (str == "slashing") type = Type::Slashing;
		else if (str == "bludgeoning") type = Type::Bludgeoning;
		else if (str == "burning") type = Type::Burning;
		else if (str == "bleeding") type = Type::Bleeding;
		else if (str == "poison") type = Type::Poison;
		else Error::fatal("Unknown damage type: " + str);
	}
	bool operator==(const Damage& other) const = default;
	bool operator!=(const Damage& other) const = default;
	bool operator<(const Damage& other) const
	{
		return amount < other.amount;
	}
	bool operator>(const Damage& other) const
	{
		return amount > other.amount;
	}
	Damage& operator+=(int add)
	{
		if (add < 0 && -add > static_cast<int>(amount))
			add = -static_cast<int>(amount);
		amount += static_cast<size_t>(add);
		return *this;
	}
	Damage& operator-=(int sub)
	{
		return operator+=(-sub);
	}
	friend Damage operator+(Damage lhs, int rhs) {
		lhs += rhs;
		return lhs;
	}

	friend Damage operator+(int lhs, Damage rhs) {
		rhs += lhs;
		return rhs;
	}
	friend Damage operator-(Damage lhs, int rhs) {
		lhs -= rhs;
		return lhs;
	}

	friend Damage operator-(int lhs, Damage rhs) {
		rhs -= lhs;
		return rhs;
	}
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
