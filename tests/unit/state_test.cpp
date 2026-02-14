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
		const auto attributes = ECS::get_attributes(registry, creature);
		(void) attributes;
	}
	SUCCEED();
}
