// Dice.cpp

#include "domain/Dice.hpp"

#include <charconv>
#include <cctype>
#include <stdexcept>
#include <string_view>

namespace
{

	[[noreturn]]
		void throw_invalid_dice(const std::string_view expression)
		{
			throw std::invalid_argument{
				"Invalid dice expression: " +
					std::string{expression}};
		}

	int parse_integer(
			std::string_view text,
			const std::string_view original_expression)
	{
		if (text.empty())
			throw_invalid_dice(original_expression);

		// std::from_chars does not accept a leading '+'.
		if (text.front() == '+')
		{
			text.remove_prefix(1);

			if (text.empty())
				throw_invalid_dice(original_expression);
		}

		int result{};

		const auto [end, error] = std::from_chars(
				text.data(),
				text.data() + text.size(),
				result);

		if (error != std::errc{} ||
				end != text.data() + text.size())
		{
			throw_invalid_dice(original_expression);
		}

		return result;
	}

} // namespace

Dice::Dice(const std::string& expression)
{
	std::string text;
	text.reserve(expression.size());

	for (const unsigned char character : expression)
	{
		if (!std::isspace(character))
			text.push_back(static_cast<char>(character));
	}

	if (text.empty())
		throw_invalid_dice(expression);

	const auto d_position = text.find_first_of("dD");

	// A number without a die is a constant value.
	if (d_position == std::string::npos)
	{
		amount = 0;
		sides = 0;
		bonus = parse_integer(text, expression);
		return;
	}

	// Detect another 'd'.
	if (text.find_first_of("dD", d_position + 1) !=
			std::string::npos)
	{
		throw_invalid_dice(expression);
	}

	if (d_position == 0)
	{
		amount = 1;
	}
	else
	{
		amount = parse_integer(
				std::string_view{text}.substr(0, d_position),
				expression);
	}

	const auto remainder =
		std::string_view{text}.substr(d_position + 1);

	if (remainder.empty())
		throw_invalid_dice(expression);

	const auto bonus_position =
		remainder.find_first_of("+-");

	const auto sides_text =
		remainder.substr(0, bonus_position);

	sides = parse_integer(sides_text, expression);

	if (bonus_position != std::string_view::npos)
	{
		bonus = parse_integer(
				remainder.substr(bonus_position),
				expression);
	}

	if (amount <= 0 || sides <= 0)
		throw_invalid_dice(expression);
}

int Dice::roll(const int advantage) const
{
	if (amount == 0 || sides == 0)
		return 0;
	int result = 0;
	for (size_t i = 0; i < amount; ++i)
		result += Random::rand<size_t>(1, sides);

	for (int i = advantage; i != 0; i += advantage / std::abs(advantage))
	{
		int other_result = roll();
		if (other_result < result && advantage < 0)
			result = other_result;
		if (other_result > result && advantage > 0)
			result = other_result;
	}

	return result + bonus;
}
