#pragma once

#include <ncurses.h>
#include <panel.h>
#include "World.hpp"
#include "Utils.hpp"
#include "entt.hpp"

class Game
{
	private:
		World world;
	public:
		Cave& get_cave() { return world.get_cave(level); }
		entt::registry& get_registry() { return world.get_registry(); }

	private:
		size_t level;
	public:
		void set_level(const size_t level) { this->level = level; }
		size_t get_level() const { return level; }

	private:
		entt::entity player;
	public:
		void set_player(const entt::entity player) { this->player = player; }
		entt::entity get_player() const { return player; }

	public:
		Game();
		void loop();
		void end();
		void check_change_level();
};

void new_game();
