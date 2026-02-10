#include <gtest/gtest.h>
#include <nlohmann/json.hpp>
#include "helpers.hpp"
#include "systems/state/StateSystem.hpp"
#include "domain/Attribute.hpp"
#include "external/entt/entt.hpp"
#include "database/EntityFactory.hpp"

TEST_F(RegistryTest, AllCreaturesHaveAllAttributes)
{
	const auto creatures = EntityFactory::instance()
		.create_entities(
				registry,
				nlohmann::json{{"category", "creatures"}}
				);
	for (const auto creature : creatures)
	{
		for (size_t i = static_cast<size_t>(Attribute{}); i < static_cast<size_t>(Attribute::Count); ++i)
		{
			const int a = StateSystem::get_attributes_sum(registry, creature, {static_cast<Attribute>(i)});
			EXPECT_TRUE(a > 0);
		}
	}
}
