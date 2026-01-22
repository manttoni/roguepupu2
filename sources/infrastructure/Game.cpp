#include <string>                      // for basic_string, operator+, opera...
#include <vector>
#include "components/Components.hpp"
#include "database/EntityFactory.hpp"           // for EntityFactory
#include "domain/Cave.hpp"
#include "external/entt/entt.hpp"
#include "generation/CaveGenerator.hpp"
#include "infrastructure/Game.hpp"
#include "systems/action/ActionSystem.hpp"
#include "systems/environment/LiquidSystem.hpp"
#include "utils/ECS.hpp"
#include "utils/Log.hpp"
#include "utils/Parser.hpp"

Game::Game()
{
	ECS::init_registry(registry);
	registry.ctx().get<GameState>().player = EntityFactory::instance().create_entity(registry, "rabdin");
	const size_t cave_idx = registry.ctx().get<World>().new_cave();
	CaveGenerator::Data data(registry, ECS::get_cave(registry, cave_idx));
	Parser::parse_cave_generation_conf("default", data);
	CaveGenerator::generate_cave(data);
	const auto sources = data.cave.get_positions_with_type(Cell::Type::Source);
	registry.emplace<Position>(ECS::get_player(registry), sources[0]);
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

