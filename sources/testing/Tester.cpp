#include <string>
#include <iostream>
#include <fstream>
#include "systems/VisionSystem.hpp"
#include "EntityFactory.hpp"
#include "Tester.hpp"
#include "Utils.hpp"
#include "entt.hpp"
#include "World.hpp"
#include "Cave.hpp"
#include "Cell.hpp"
#include "ECS.hpp"

namespace Tester
{
	Results test_results{};

	void log(const bool success, const std::string& message)
	{
		std::ofstream logfile("logs/tester.log", std::ios::app);
		if (!logfile)
			Log::error("Can't open tester.log");

		logfile << Log::timestamp() << " ";
		logfile << "[" << (success ? "Success" : "Fail") << "] ";
		logfile << message;
		logfile.close();
	}

	void test_assert(const bool condition, const std::string& message)
	{
		if (condition == false)
			test_results.failed_tests++;
		if (condition == false || test_results.log_success)
			log(condition, message);
	}
/*
	size_t test_glowing_mushroom()
	{
		entt::registry registry = ECS::init_registry();
		Cave test_cave = Cave(10, Cell::Type::Floor);
		const size_t cave_idx = registry.ctx().get<World>().add_cave(test_cave);
		//const size_t cell_idx = 55;
		const Position& position = {.cell_idx = 55, .cave_idx = cave_idx};

		//const auto mushroom = EntityFactory::instance()
		//	.create_entity(registry, "glowing mushroom", position);
		for (const auto& cell : test_cave.get_cells())
		{
			if (VisionSystem::has_line_of_sight(registry, cave_idx, cell_idx, cell) &&
					PositionSystem::distance(test_cave, spawn_cell, cell) <= glow_radius)
				test_assert(LightingSystem::get_illumination(cell) > 0, "Cell should be illuminated");
			else
				test_assert(LightingSystem::get_illumination(cell) == 0, "Cell should be dark");
		}
	}
*/

	bool test()
	{
	//	test_glowing_mushroom();

		log(test_results.failed_tests == 0, "Failed tests: " + std::to_string(test_results.failed_tests));
		std::cout << "Failed tests: " << test_results.failed_tests << std::endl;
		return test_results.failed_tests != 0;
	}
};
