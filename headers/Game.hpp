#pragma once

#include "GameLogger.hpp"  // for GameLogger
#include "World.hpp"       // for World
#include "entt.hpp"        // for entity, size_t, registry
class Cave;

class Game
{
	private:
		World world;
		GameLogger log;
		size_t level;
		entt::entity player;
	public:
		GameLogger& get_log() { return log; }
		Cave& get_cave() { return world.get_cave(level); }
		entt::registry& get_registry() { return world.get_registry(); }
		void set_level(const size_t level) { this->level = level; }
		size_t get_level() const { return level; }
		void set_player(const entt::entity player) { this->player = player; }
		entt::entity get_player() const { return player; }
		Game();
		void handle_key(const int key);
		void environment_turn();
		void reset_actions();
		void loop();
		void end();
		void check_change_level();
};

void new_game();
