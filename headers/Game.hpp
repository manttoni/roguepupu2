#pragma once

#include <ncurses.h>
#include <panel.h>
#include "CaveGenerator.hpp"
#include "Utils.hpp"

class Game
{
	private:
		CaveGenerator cavegen;
		Cave& current_cave;
	public:
		Game();
		void start();
		void end();
		double move_with_mouse();
		double move_player(const Vec2& direction);
		void action_menu();
};

void new_game();
