#pragma once

#include "GameState.hpp"
#include "GameLogger.hpp"  // for GameLogger
#include "World.hpp"       // for World
#include "entt.hpp"        // for entity, size_t, registry
#include "AbilityDatabase.hpp"
class Cave;

class Game
{
	private:
		World world;
		entt::registry& registry;
		entt::entity player;
		bool game_over = false;
	public:
		bool is_over() const { return game_over; }
		Game();
		void loop();
		void end();
};

void new_game();
