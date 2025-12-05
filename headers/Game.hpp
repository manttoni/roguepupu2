#pragma once

#include <ncurses.h>
#include <panel.h>
#include "CaveGenerator.hpp"
#include "Utils.hpp"

class Game
{
	private:
		CaveGenerator cavegen;
		Cave* current_cave;
	public:
		Game();
		void start();
		void end();
		double move_player(const Vec2& direction);
		void check_descend();
		void change_level(const int d);
		entt::entity get_player();
};

void new_game();
