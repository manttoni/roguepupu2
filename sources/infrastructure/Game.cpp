#include <string>                      // for basic_string, operator+, opera...
#include "infrastructure/Game.hpp"
#include "systems/ActionSystem.hpp"
#include "database/EntityFactory.hpp"           // for EntityFactory
#include "systems/EnvironmentSystem.hpp"

Game::Game() :
	registry(ECS::init_registry())
{
	registry.ctx().get<GameState>().player = EntityFactory::instance().create_entity(registry, "rabdin");
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

