#include <string>
#include <cassert>
#include <iostream>
#include <fstream>
#include "systems/rendering/RenderingSystem.hpp"
#include "UI/UI.hpp"
#include "systems/perception/VisionSystem.hpp"
#include "utils/Vec2.hpp"
#include "systems/rendering/LightingSystem.hpp"
#include "database/EntityFactory.hpp"
#include "testing/Tester.hpp"
#include "domain/World.hpp"
#include "domain/Cave.hpp"
#include "domain/Cell.hpp"
#include "utils/ECS.hpp"
#include "utils/Log.hpp"
#include "systems/action/EventSystem.hpp"

namespace Tester
{
	void log(const bool success, const std::string& message)
	{
		std::ofstream logfile("logs/tester.log", std::ios::app);
		if (!logfile)
			Log::error("Can't open tester.log");

		logfile << Log::timestamp() << " ";
		logfile << "[" << (success ? "Success" : "Fail") << "] ";
		logfile << message << std::endl;
		logfile.close();
	}

	size_t get_test_cave_idx(entt::registry& registry, const size_t size)
	{
		const auto cave_idx = ECS::get_world(registry).new_cave(size, Cell::Type::Rock);
		Cave& cave = ECS::get_cave(registry, cave_idx);
		for (const auto pos : cave.get_positions())
		{
			const Vec2 coords(pos.cell_idx, cave.get_size());
			if (coords.y == 0 || static_cast<size_t>(coords.y) == cave.get_size() - 1 ||
					coords.x == 0 || static_cast<size_t>(coords.x) == cave.get_size() - 1)
				continue;
			cave.get_cell(pos).set_type(Cell::Type::Floor);
		}
		return cave_idx;
	}

	size_t test_glowing_position(entt::registry& registry, const Position& pos)
	{
		size_t failed = 0;
		auto& cave = ECS::get_cave(registry, pos.cave_idx);
		auto& cell = ECS::get_cell(registry, pos);
		const auto mushroom = EntityFactory::instance().create_entity(registry, "glowing mushroom", pos);

		assert(registry.ctx().get<EventQueue>().queue.size() > 0);
		EventSystem::resolve_events(registry);
		assert(registry.ctx().get<EventQueue>().queue.empty());

		auto lights = cell.get_lights();
		if (lights.size() != 1)
		{
			failed++;
			log(false, "Expected one stack of lights on glowing mushroom, but was: " + std::to_string(lights.size()));
		}
		assert(lights.size() == 1);
		assert(lights.begin()->first.get_illumination() > 0);

		const auto& glow = registry.get<Glow>(mushroom);
		const auto& fgcolor = registry.get<FGColor>(mushroom);
		const auto expected_illumination = fgcolor.color.get_illumination() * glow.intensity;
		const auto expected_color = fgcolor.color * glow.intensity;

		for (const auto nearby : cave.get_nearby_positions(pos, glow.radius))
		{
			const auto& nearby_cell = cave.get_cell(nearby);
			const auto illumination = LightingSystem::get_illumination(nearby_cell);

			if (!VisionSystem::has_line_of_sight(registry, nearby, pos))
			{
				if (illumination != 0.0)
				{
					failed++;
					log(false, "Expected no light here, but was: " + std::to_string(illumination));
				}
				continue;
			}

			const auto nearby_lights = nearby_cell.get_lights();
			if (nearby_lights.size() != 1)
			{
				failed++;
				log(false, "Cell should have only one light stack, but has " + std::to_string(nearby_lights.size()));
			}
			assert(nearby_lights.begin()->first != Color{});
			const auto color = nearby_lights.begin()->first;
			if (color != expected_color)
			{
				failed++;
				log(false, "Expected color around mushroom: " + expected_color.markup() +
						", but was: " + color.markup());
			}
			if (illumination != expected_illumination)
			{
				failed++;
				log(false, "Expected light around mushroom: " +
						std::to_string(expected_illumination) +
						", but was: " + std::to_string(illumination));
			}
		}
		registry.destroy(mushroom);
		return failed;
	}

	size_t test_glowing_mushroom()
	{
		size_t failed = 0;
		entt::registry registry;
		ECS::init_registry(registry);
		const auto cave_idx = get_test_cave_idx(registry, 10);
		auto& cave = ECS::get_cave(registry, cave_idx);
		for (const auto pos : cave.get_positions_with_type(Cell::Type::Floor))
			failed += test_glowing_position(registry, pos);
		return failed;
	}


	void test()
	{
		size_t failed = test_glowing_mushroom();
		std::cout << "Failed: " << failed << std::endl;
	}
};
