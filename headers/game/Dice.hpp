#pragma once

#include <ostream>
#include <string>
#include <nlohmann/json_fwd.hpp>
#include <vector>
#include "utils/Random.hpp"

namespace Game
{
	struct Dice
	{
		int amount;
		int sides;
		int bonus;

		Dice() = default;
		Dice(const std::string& str);

		int roll(const int advantage = 0) const;
		std::string to_string() const;

		bool operator==(const Dice& other) const = default;
		bool operator!=(const Dice& other) const = default;
	};
	inline std::ostream& operator<<(std::ostream& os, const Dice& dice)
	{
		return os << dice.to_string();
	}
	void to_json(nlohmann::json& data, const Dice& dice);
	void from_json(const nlohmann::json& data, Dice& dice);
} // namespace Game
