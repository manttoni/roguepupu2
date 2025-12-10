#include <iostream>
#include <stdexcept>
#include <nlohmann/json.hpp>
#include <vector>
#include <string>
#include <map>
#include "ECS.hpp"
#include "EntityFactory.hpp"
#include "Utils.hpp"
#include "entt.hpp"
#include "systems/EquipmentSystem.hpp"


size_t test_equipping_all_weapons()
{
	size_t failed = 0;
	Log::tester_log(Log::Type::INFO, "Starting test_equipping_all_weapons");
	nlohmann::json filter = {{"subcategory", "weapons"}};
	const std::vector<std::string>& all_weapon_names = EntityFactory::instance().random_pool(filter, SIZE_MAX);

	// Key is name of tester entity, value is expected amount of succesful equippings
	// Failed equipping happens when they don't have proficiency
	const std::map<std::string, size_t> tests =
	{
		{"rabdin", all_weapon_names.size()} // has to be able to equip any item (at least now)
	};

	entt::registry test_registry;

	for (const auto& [tester_name, expected_equippings] : tests)
	{
		const auto test_entity = EntityFactory::instance().create_entity(test_registry, tester_name);
		for (const auto& weapon_name : all_weapon_names)
		{
			const auto test_weapon = EntityFactory::instance().create_entity(test_registry, weapon_name);
			if (ECS::has_weapon_property(test_registry, test_weapon, "versatile") && ECS::get_versatile_dice(test_registry, test_weapon).get_string().empty())
			{
				Log::tester_log(Log::Type::TEST_FAIL, "Expected a versatile weapon to have versatile dice");
				failed++;
			}
			EquipmentSystem::equip(test_registry, test_entity, test_weapon);
			if (!EquipmentSystem::is_equipped(test_registry, test_entity, test_weapon))
			{
				Log::tester_log(Log::Type::TEST_FAIL, "Expected " + weapon_name + " to be equipped, but it's not");
				failed++;
			}
		}
	}
	Log::tester_log(Log::Type::INFO, "Finished test_equipping_all_weapons with " + std::to_string(failed) + " failed tests");
	return failed;
}

size_t tester()
{
	size_t failed = 0;
	Log::tester_log(Log::Type::INFO, "-- Starting tester --");
	failed += test_equipping_all_weapons();
	Log::tester_log(Log::Type::INFO, "Tester finished with " + std::to_string(failed) + " failed tests");
	std::cout << "Tester finished with " << failed << " failed tests" << std::endl;
	return failed;
}
