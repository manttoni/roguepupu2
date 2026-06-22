#include <gtest/gtest.h>
#include <string>
#include <vector>

#include "helpers.hpp"
#include "database/EntityFactory.hpp"
#include "systems/combat/DamageSystem.hpp"
#include "systems/state/EquipmentSystem.hpp"
#include "domain/Damage.hpp"
#include "external/entt/entt.hpp"
#include "infrastructure/EventLogger.hpp"
#include "systems/action/EventSystem.hpp"
#include "components/Components.hpp"
#include "gtest/gtest.h"
#include "utils/ECS.hpp"
#include "utils/Utils.hpp"
#include "utils/Log.hpp"

TEST_F(RegistryTest, EntityTakesDamage)
{
	const auto creature = EntityFactory::instance().create_entity(registry, "test_creature");

	ASSERT_TRUE(creature != entt::null);
	ASSERT_TRUE(registry.all_of<HitPoints>(creature));

	const Damage::Roll damage(Damage::Type::Piercing, 1);
	const auto health_before = registry.get<HitPoints>(creature);
	DamageSystem::take_damage(registry, creature, damage);
	const auto health_after = registry.get<HitPoints>(creature);
	EXPECT_NE(health_before.value, health_after.value);
}

TEST_F(RegistryTest, TakingDamageUpdatesLog)
{
	Log::debug() << "Damage test start";
	const auto creature = EntityFactory::instance().create_entity(registry, "test_creature");

	const Damage::Roll damage(Damage::Type::Piercing, 1);

	// DamageSystem will queue an event after removing hp
	DamageSystem::take_damage(registry, creature, damage);

	auto size2 = registry.ctx().get<EventQueue>().queue.size();

	Log::debug() << size2 << " events in queue";
	EventSystem::resolve_events(registry);

	auto size = registry.ctx().get<EventLogger>().get_last_events(1).size();
	Log::debug() << size << " events in last 1 events in logger";

	ASSERT_TRUE(size > 0);

	// EventSystem will log a message with the damage info

	const auto logger = registry.ctx().get<EventLogger>();
	const auto messages = logger.get_last_messages(30);
	const std::string expected_message = ECS::get_colored_name(registry, creature) + " takes 1 piercing damage";
	for (const auto& message : messages)
	{
		if (message == expected_message)
		{
			Log::debug() << "Damage test end success";
			SUCCEED();
			return;
		}
	}

	Log::debug() << "Damage test end fail";
	FAIL() << "Did not find expected message, found " << messages.size() << " messages";
}


