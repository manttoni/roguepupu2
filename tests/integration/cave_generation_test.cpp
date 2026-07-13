#include <gtest/gtest.h>
#include "helpers.hpp"
#include "generation/CaveGenerator.hpp"

TEST_F(RegistryTest, GenerateCave)
{
	// when registry is initialized it will get blank caves, and there is at least one with index 0
	CaveGenerator::generate(registry, 0);
	SUCCEED();
}
