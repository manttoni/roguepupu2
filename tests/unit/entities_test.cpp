#include <gtest/gtest.h>
#include "helpers.hpp"

#include "external/entt/entt.hpp"
#include "utils/ECS.hpp"

TEST_F(RegistryTest, CreateAllEntities)
{
	// Test if can create all entities
	// Error might also be in entity definitions data
	const auto ids = ECS::get_entity_ids(registry);
	for (const auto& id : ids)
	{
		const auto entity = ECS::create_entity(registry, id);
		EXPECT_TRUE(registry.valid(entity));
	}
}
