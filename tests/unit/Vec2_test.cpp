#include <gtest/gtest.h>
#include "utils/Vec2.hpp"

TEST(Vec2Test, IntOperatorPlusEquals)
{
	Vec2<int> a(0, 0);
	Vec2<int> b(1, 2);
	EXPECT_EQ(0, a.y);
	EXPECT_EQ(0, a.x);
	EXPECT_EQ(1, b.y);
	EXPECT_EQ(2, b.x);
	a += b;
	EXPECT_EQ(1, a.y);
	EXPECT_EQ(2, a.x);
	EXPECT_EQ(1, b.y);
	EXPECT_EQ(2, b.x);
}

TEST(Vec2Test, IntOperatorMultiplyScalar)
{
	Vec2<int> a(3, 4);
	Vec2<int> b(1, 2);
	EXPECT_EQ(3, a.y);
	EXPECT_EQ(4, a.x);
	EXPECT_EQ(1, b.y);
	EXPECT_EQ(2, b.x);
	auto c = a * 2;
	EXPECT_EQ(3, a.y);
	EXPECT_EQ(4, a.x);
	EXPECT_EQ(1, b.y);
	EXPECT_EQ(2, b.x);
	EXPECT_EQ(6, c.y);
	EXPECT_EQ(8, c.x);
}
