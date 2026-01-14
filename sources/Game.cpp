#include <string>                      // for basic_string, operator+, opera...
#include "Game.hpp"
#include "systems/ActionSystem.hpp"
#include "EntityFactory.hpp"           // for EntityFactory
#include "GameLogger.hpp"              // for GameLogger
#include "Utils.hpp"                   // for log, top, Vec2
#include "entt.hpp"                    // for vector, allocator, basic_sigh_...
#include "systems/EnvironmentSystem.hpp"
#include "ECS.hpp"

Game::Game() :
	registry(ECS::init_registry()),
	player(EntityFactory::instance().create_entity(registry, "rabdin"))
{
	Log::log("Game object constructed");
}

void Game::loop()
{
	Log::log("Game loop started");
	registry.ctx().get<GameState>().running = true;
	while (registry.ctx().get<GameState>().running)
	{
		Log::log("Round " + std::to_string(registry.ctx().get<GameState>().turn_number));
		ActionSystem::act_round(registry);
		EnvironmentSystem::simulate_environment(ECS::get_active_cave(registry));
		registry.ctx().get<GameState>().turn_number++;
	}
	if (registry.all_of<Dead>(player))
		game_over = true;
}

