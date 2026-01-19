#include <string>                      // for basic_string, operator+, opera...
#include <vector>
#include "testing/DevTools.hpp"
#include "components/Components.hpp"
#include "database/AbilityDatabase.hpp"
#include "database/EntityFactory.hpp"           // for EntityFactory
#include "domain/Cave.hpp"
#include "domain/Cell.hpp"
#include "domain/World.hpp"
#include "external/entt/entt.hpp"
#include "generation/CaveGenerator.hpp"
#include "infrastructure/Game.hpp"
#include "infrastructure/GameLogger.hpp"
#include "infrastructure/GameState.hpp"
#include "systems/action/ActionSystem.hpp"
#include "systems/environment/LiquidSystem.hpp"
#include "utils/ECS.hpp"
#include "utils/Log.hpp"
#include "systems/rendering/RenderData.hpp"
#include "utils/Parser.hpp"

Game::Game()
{
	Log::log("Constructing game object");
	registry.ctx().emplace<GameState>();
	registry.ctx().emplace<GameLogger>();
	registry.ctx().emplace<AbilityDatabase>();
	registry.ctx().emplace<World>();
	registry.ctx().emplace<RenderData>();
	registry.ctx().emplace<EventQueue>();
	registry.ctx().emplace<Dev>();
	Log::log("Emplaced ctx objects");

	registry.ctx().get<GameState>().player = EntityFactory::instance().create_entity(registry, "rabdin");

	Log::log("Created and added player to GameState");

	const size_t cave_idx = registry.ctx().get<World>().new_cave();

	Log::log("Added new cave to World with idx " + std::to_string(cave_idx));

	CaveGenerator::Data data(registry, ECS::get_cave(registry, cave_idx));
	Parser::parse_cave_generation_conf("default", data);

	Log::log("Constructed Data");

	CaveGenerator::generate_cave(data);

	Log::log("Cave generated");

	const auto sources = data.cave.get_positions_with_type(Cell::Type::Source);

	Log::log("Cave has sources: " + std::to_string(sources.size()));

	registry.emplace<Position>(ECS::get_player(registry), sources[0]);
	registry.emplace<Perception>(ECS::get_player(registry), 10);

	Log::log("Emplaced position to rabdin");

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
		LiquidSystem::simulate_liquids(registry);
		registry.ctx().get<GameState>().turn_number++;
	}
	if (registry.all_of<Dead>(ECS::get_player(registry)))
		game_over = true;
}

