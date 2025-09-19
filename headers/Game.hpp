#pragma once

#include <ncurses.h>
#include <panel.h>
#include "CaveGenerator.hpp"
#include "Creature.hpp"

class Game
{
	private:
		CaveGenerator cavegen;
		void init_cavegen();
		void init_panels();
		void draw_cave(Cave& cave) const;
		void draw_cell(const Cell& cell) const;
		size_t level;
		Creature player;
	public:
		Game();
		void start();
};

void start_game();
