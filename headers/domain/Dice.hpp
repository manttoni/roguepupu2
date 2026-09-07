#pragma once

#include <ostream>
#include <string>
#include <vector>
#include "utils/Random.hpp"

namespace Domain
{

struct Dice
{
	size_t amount = 0;
	size_t sides = 0;
	size_t bonus = 0;

	Dice(const size_t amount = 0, const size_t sides = 0, const size_t bonus = 0) : amount(amount), sides(sides), bonus(bonus) {}
	Dice(const std::string& str);

	int roll(const int advantage = 0) const;
	inline bool is_valid() const { return amount > 0 && sides > 0; }

	bool operator==(const Dice& other) const = default;
	bool operator!=(const Dice& other) const = default;
};

inline std::ostream& operator<<(std::ostream& os, const Dice& dice)
{
	os << dice.amount << "d" << dice.sides << "+" << dice.bonus;
	return os;
}
} // namespace Domain
