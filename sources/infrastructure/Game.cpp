#include <stddef.h>
#include <string>                      // for basic_string, operator+, opera...
#include <vector>

#include "infrastructure/CharacterCreation.hpp"
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
#include "UI/Dialog.hpp"
#include "domain/Cell.hpp"
#include "domain/Position.hpp"
#include "domain/World.hpp"
#include "infrastructure/GameState.hpp"

struct Dead;

Game::Game()
{
	ECS::init_registry(registry);

	const auto choice = Dialog::get_selection("Create custom character?", {"No", "Yes"}).label;
	if (choice == "No")
		registry.ctx().get<GameState>().player = EntityFactory::instance().create_entity(registry, "rabdin");
	else if (choice == "Yes")
		registry.ctx().get<GameState>().player = CharacterCreation::create_character(registry);
	else
		game_over = true;

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
	const auto player = ECS::get_player(registry);
	while (registry.ctx().get<GameState>().running && game_over == false)
	{
		Log::log("Round " + std::to_string(registry.ctx().get<GameState>().turn_number));
		ActionSystem::act_round(registry, ECS::get_cave(registry, registry.get<Position>(player)).get_idx());
		LiquidSystem::simulate_liquids(registry);
		registry.ctx().get<GameState>().turn_number++;
	}
	if (registry.all_of<Dead>(player))
		game_over = true;
}

