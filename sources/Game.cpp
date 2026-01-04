#include <thread>
#include <chrono>
#include <string>                      // for basic_string, operator+, opera...
#include "systems/ActionSystem.hpp"
#include "Cave.hpp"                    // for Cave
#include "Cell.hpp"                    // for Cell
#include "Components.hpp"              // for Actions, Position
#include "ECS.hpp"                     // for get_cell, can_see, get_colored...
#include "EntityFactory.hpp"           // for EntityFactory
#include "Game.hpp"                    // for Game
#include "GameLogger.hpp"              // for GameLogger
#include "Renderer.hpp"                // for Renderer
#include "UI.hpp"                      // for UI, KEY_ESCAPE, KEY_LEFT_CLICK
#include "Utils.hpp"                   // for log, top, Vec2
#include "World.hpp"                   // for World
#include "entt.hpp"                    // for vector, allocator, basic_sigh_...
#include "systems/ContextSystem.hpp"   // for show_entities_list, show_entit...
#include "systems/MovementSystem.hpp"  // for move

Game::Game() :
	registry(world.get_registry()),
	player(EntityFactory::instance().create_entity(registry, "rabdin", &world.get_cave(1).get_source()))
{
	registry.ctx().emplace<GameLogger>();
	registry.ctx().emplace<GameState>();
	registry.ctx().emplace<Renderer>(registry);
	registry.ctx().emplace<EventQueue>();
	Log::log("Game object constructed");
}

void Game::loop()
{
	Log::log("Game loop started");
	registry.ctx().get<GameState>().running = true;
	while (registry.ctx().get<GameState>().running)
	{
		ActionSystem::act_round(registry);
		registry.ctx().get<GameState>().turn_number++;
	}
	if (registry.all_of<Dead>(player))
		game_over = true;
}

