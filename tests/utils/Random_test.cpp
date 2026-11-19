#include <array>
#include <stdexcept>

#include <gtest/gtest.h>

#include "utils/Random.hpp"

namespace
{
	TEST(RandomAtTest, RepeatsForTheSameSeedPositionAndStream)
	{
		const auto seed = Random::hash_seed("ruins");
		const Game::World::GlobalPosition position{-42, 19};
		const double original = Random::at(seed, position, 7);

		Random::seed(190);
		for (int i = 0; i < 12; ++i)
			(void) Random::integer(1, 20);

		EXPECT_EQ(Random::at(seed, position, 7), original);
		EXPECT_EQ(Random::at(seed, position), Random::at(seed, position, 0));
	}

	TEST(RandomAtTest, SeedBothCoordinatesAndStreamAffectTheRoll)
	{
		const auto seed = Random::hash_seed("ruins");
		const Game::World::GlobalPosition position{-42, 19};
		const double original = Random::at(seed, position, 7);

		EXPECT_NE(Random::at(Random::hash_seed("cave"), position, 7), original);
		EXPECT_NE(Random::at(seed, {-43, 19}, 7), original);
		EXPECT_NE(Random::at(seed, {-42, 20}, 7), original);
		EXPECT_NE(Random::at(seed, position, 8), original);
	}

	TEST(RandomAtTest, ReturnsValuesInTheHalfOpenUnitInterval)
	{
		const auto seed = Random::hash_seed("range");
		for (int y = -20; y <= 20; ++y)
		{
			for (int x = -20; x <= 20; ++x)
			{
				const auto result = Random::at(seed, {y, x}, 3);
				ASSERT_GE(result, 0.0);
				ASSERT_LT(result, 1.0);
			}
		}
	}

	TEST(RandomIntegerTest, ReseedingReplaysTheSequence)
	{
		std::array<int, 16> first{};
		Random::seed(739);
		for (auto& roll : first)
			roll = Random::integer(-25, 25);

		Random::seed(739);
		for (const auto roll : first)
			EXPECT_EQ(Random::integer(-25, 25), roll);
	}

	TEST(RandomIntegerTest, IncludesBothBoundsAndAcceptsASingleValue)
	{
		Random::seed(52);
		bool saw_minimum = false;
		bool saw_maximum = false;
		for (int i = 0; i < 128; ++i)
		{
			const auto roll = Random::integer(-1, 0);
			ASSERT_GE(roll, -1);
			ASSERT_LE(roll, 0);
			saw_minimum |= roll == -1;
			saw_maximum |= roll == 0;
		}

		EXPECT_TRUE(saw_minimum);
		EXPECT_TRUE(saw_maximum);
		EXPECT_EQ(Random::integer(4, 4), 4);
		EXPECT_THROW(Random::integer(1, 0), std::invalid_argument);
	}
}
