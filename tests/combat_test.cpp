#include <gtest/gtest.h>
#include <string>
#include <vector>

#include "helpers.hpp"
#include "database/EntityFactory.hpp"
#include "systems/combat/DamageSystem.hpp"
#include "domain/Damage.hpp"
#include "external/entt/entt.hpp"
#include "infrastructure/GameLogger.hpp"
#include "systems/action/EventSystem.hpp"
#include "components/Components.hpp"
#include "gtest/gtest.h"
#include "utils/ECS.hpp"

TEST_F(RegistryTest, EntityTakesDamage)
{
	const auto dummy = EntityFactory::instance().create_entity(registry, "test_dummy");

	ASSERT_TRUE(dummy != entt::null);
	ASSERT_TRUE(registry.all_of<Health>(dummy));

	const Damage damage(Damage::Type::Piercing, 1);
	const auto health_before = registry.get<Health>(dummy);
	DamageSystem::take_damage(registry, dummy, damage);
	const auto health_after = registry.get<Health>(dummy);
	EXPECT_NE(health_before, health_after);
}

TEST_F(RegistryTest, TakingDamagePrintsLog)
{
	const auto dummy = EntityFactory::instance().create_entity(registry, "test_dummy");

	const Damage damage(Damage::Type::Piercing, 1);

	// DamageSystem will queue an event after removing hp
	DamageSystem::take_damage(registry, dummy, damage);

	// EventSystem will log a message with the damage info
	EventSystem::resolve_events(registry);

	const auto logger = registry.ctx().get<GameLogger>();
	const auto last_message = logger.last(1).front();

	ASSERT_FALSE(last_message.empty());

	const std::string expected_message = ECS::get_colored_name(registry, dummy) + " takes " + damage.to_string() + " damage";

	EXPECT_EQ(expected_message, last_message);
}
