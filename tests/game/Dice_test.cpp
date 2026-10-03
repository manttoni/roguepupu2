#include <gtest/gtest.h>

#include <sstream>
#include <stdexcept>

#include "game/Dice.hpp"

TEST(DiceParsing, ParsesAmountSidesAndBonus)
{
	const Game::Dice dice("2d6+3");

	EXPECT_EQ(dice.amount, 2);
	EXPECT_EQ(dice.sides, 6);
	EXPECT_EQ(dice.bonus, 3);
}

TEST(DiceParsing, DefaultsToOneDie)
{
	const Game::Dice dice("d20");

	EXPECT_EQ(dice.amount, 1);
	EXPECT_EQ(dice.sides, 20);
	EXPECT_EQ(dice.bonus, 0);
}

TEST(DiceParsing, SupportsWhitespaceAndNegativeBonus)
{
	const Game::Dice dice(" 2D6 - 3 ");

	EXPECT_EQ(dice.amount, 2);
	EXPECT_EQ(dice.sides, 6);
	EXPECT_EQ(dice.bonus, -3);
}

TEST(DiceParsing, ParsesConstants)
{
	const Game::Dice dice("-5");

	EXPECT_EQ(dice.amount, 0);
	EXPECT_EQ(dice.sides, 0);
	EXPECT_EQ(dice.bonus, -5);
}

TEST(DiceParsing, RejectsInvalidExpressions)
{
	for (const char* expression : {
			"", "abc", "d", "2d", "0d6", "-1d6",
			"1d0", "1d-6", "2d6+", "2d6+-3", "2d6junk",
			"999999999999999999999d6"
		})
	{
		SCOPED_TRACE(expression);
		EXPECT_THROW(
				(void)Game::Dice(expression),
				std::invalid_argument);
	}
}

TEST(DiceRoll, ReturnsConstants)
{
	EXPECT_EQ(Game::Dice("5").roll(), 5);
	EXPECT_EQ(Game::Dice("-2").roll(), -2);
	EXPECT_EQ(Game::Dice("0").roll(), 0);
}

TEST(DiceRoll, AppliesBonusExactlyOnceWithEitherAdvantage)
{
	// One-sided dice make the result deterministic.
	for (const int advantage : {-2, -1, 0, 1, 2})
	{
		SCOPED_TRACE(advantage);

		EXPECT_EQ(Game::Dice("2d1+3").roll(advantage), 5);
		EXPECT_EQ(Game::Dice("2d1-3").roll(advantage), -1);
	}
}

TEST(DiceRoll, StaysWithinPossibleRange)
{
	const Game::Dice dice("2d6-3");

	for (const int advantage : {-1, 0, 1})
	{
		SCOPED_TRACE(advantage);

		for (int i = 0; i < 100; ++i)
		{
			const int result = dice.roll(advantage);

			EXPECT_GE(result, -1);
			EXPECT_LE(result, 9);
		}
	}
}

TEST(DiceRoll, RejectsInvalidMutatedState)
{
	Game::Dice dice("1d6");
	dice.sides = 0;

	EXPECT_THROW(dice.roll(), std::invalid_argument);
}

TEST(DiceRoll, DetectsResultOverflow)
{
	EXPECT_THROW(
			Game::Dice("1d1+2147483647").roll(),
			std::overflow_error);
}

TEST(DiceFormatting, ProducesCanonicalExpressions)
{
	EXPECT_EQ(Game::Dice("d20").to_string(), "1d20");
	EXPECT_EQ(Game::Dice("2d6+3").to_string(), "2d6+3");
	EXPECT_EQ(Game::Dice("2d6-3").to_string(), "2d6-3");
	EXPECT_EQ(Game::Dice("2d6+0").to_string(), "2d6");
	EXPECT_EQ(Game::Dice("-5").to_string(), "-5");
}

TEST(DiceFormatting, SupportsStreamOutput)
{
	std::ostringstream output;
	output << Game::Dice("2d6-3");

	EXPECT_EQ(output.str(), "2d6-3");
}
