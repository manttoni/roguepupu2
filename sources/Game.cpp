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
	level(1),
	registry(world.get_registry()),
	player(EntityFactory::instance().create_entity(registry, "rabdin", &get_cave().get_source()))
{
	registry.ctx().emplace<GameLogger>(game_log);
	registry.ctx().emplace<GameState>(game_state);
}

void Game::loop()
{
	registry.ctx().get<GameState>().running = true;
	while (registry.ctx().get<GameState>().running)
	{
		ActionSystem::npc_turn(registry);
		ActionSystem::player_turn(registry);
		registry.ctx().get<GameState>().turn_number++;
	}
	if (ECS::is_dead(registry, player))
		game_over = true;
}

