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
#include "DevTools.hpp"
#include "systems/EnvironmentSystem.hpp"

Game::Game() :
	registry(world.get_registry()),
	player(EntityFactory::instance().create_entity(registry, "rabdin"))
{
	const auto source = ECS::get_source(registry, world.get_cave(1));
	auto* source_cell = ECS::get_cell(registry, source);
	registry.emplace<Position>(player, source_cell);

	// testing
	registry.emplace<LiquidSource>(player, Liquid::Type::Blood, 0.01);

	registry.ctx().emplace<GameLogger>();
	registry.ctx().emplace<GameState>();
	registry.ctx().emplace<Renderer>(registry);
	registry.ctx().emplace<EventQueue>();
	registry.ctx().emplace<Dev>();
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

