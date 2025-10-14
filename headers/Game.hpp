#pragma once

#include <ncurses.h>
#include <panel.h>
#include "CaveGenerator.hpp"
#include "Creature.hpp"

class Game
{
	private:
		CaveGenerator cavegen;
		Cave& current_cave;
		Creature* player;
	public:
		Game();
		void start();
		double move_with_mouse();
		void action_menu();
};

void start_game();
