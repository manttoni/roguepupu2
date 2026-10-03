#include "game/Dice.hpp"

#include <algorithm>
#include <charconv>
#include <cctype>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <string_view>

namespace
{
	int parse_integer(std::string_view text)
	{
		if (text.empty())
			throw std::invalid_argument("Missing integer in dice expression");

		// from_chars accepts '-' but not '+'.
		if (text.front() == '+')
		{
			text.remove_prefix(1);

			if (text.empty() || text.front() < '0' || text.front() > '9')
				throw std::invalid_argument("Invalid integer in dice expression");
		}

		int value{};
		const auto [end, error] = std::from_chars(
				text.data(), text.data() + text.size(), value);

		if (error != std::errc{} || end != text.data() + text.size())
			throw std::invalid_argument("Invalid integer in dice expression");

		return value;
	}

	int checked_result(const std::int64_t value)
	{
		if (value < std::numeric_limits<int>::min()
				|| value > std::numeric_limits<int>::max())
		{
			throw std::overflow_error("Dice result does not fit in int");
		}

		return static_cast<int>(value);
	}
}

namespace Game
{
	Dice::Dice(const std::string& str)
		: amount(0), sides(0), bonus(0)
	{
		std::string text;
		text.reserve(str.size());

		for (const unsigned char ch : str)
		{
			if (!std::isspace(ch))
				text.push_back(static_cast<char>(ch));
		}

		const auto d = text.find_first_of("dD");

		if (d == std::string::npos)
		{
			bonus = parse_integer(text);
			return;
		}

		const std::string_view expression{text};

		amount = d == 0
			? 1
			: parse_integer(expression.substr(0, d));

		const auto remainder = expression.substr(d + 1);
		const auto modifier = remainder.find_first_of("+-");

		sides = parse_integer(remainder.substr(0, modifier));

		if (modifier != std::string_view::npos)
			bonus = parse_integer(remainder.substr(modifier));

		if (amount <= 0 || sides <= 0)
		{
			throw std::invalid_argument(
					"Dice amount and sides must be positive");
		}
	}

	int Dice::roll(const int advantage) const
	{
		// Constants have no dice.
		if (amount == 0 && sides == 0)
			return bonus;

		// Members are public, so validate again before using them.
		if (amount <= 0 || sides <= 0)
			throw std::invalid_argument("Invalid dice amount or sides");

		const auto roll_pool = [this]() -> std::int64_t
		{
			std::int64_t total = 0;

			for (int i = 0; i < amount; ++i)
				total += Random::rand<int>(1, sides);

			return total;
		};

		auto result = roll_pool();

		// Widen before negation so INT_MIN is handled safely.
		const std::int64_t extra_rolls = advantage < 0
			? -static_cast<std::int64_t>(advantage)
			: static_cast<std::int64_t>(advantage);

		for (std::int64_t i = 0; i < extra_rolls; ++i)
		{
			const auto candidate = roll_pool();

			result = advantage > 0
				? std::max(result, candidate)
				: std::min(result, candidate);
		}

		return checked_result(result + bonus);
	}

	std::string Dice::to_string() const
	{
		if (amount == 0 && sides == 0)
			return std::to_string(bonus);

		std::string result =
			std::to_string(amount) + "d" + std::to_string(sides);

		if (bonus > 0)
			result += "+";

		if (bonus != 0)
			result += std::to_string(bonus);

		return result;
	}
}
