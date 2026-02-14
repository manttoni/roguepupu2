#include <gtest/gtest.h>
#include "utils/Range.hpp"

TEST(RangeTest, ContainsWorks)
{
	const Range<double> r(0,10);
	EXPECT_TRUE(r.contains(5.0));
	const Range<double> r2(0,1.5);
	EXPECT_TRUE(r2.contains(1.0));
}
