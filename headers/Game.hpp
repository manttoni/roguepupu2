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
		void init_player();
		void draw_cave(Cave& cave);
		void draw_cell(const Cell& cell) const;
		Creature& get_player();
		size_t level;
	public:
		Game();
		void start();
};

void start_game();
