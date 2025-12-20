#pragma once

#include "Cave.hpp"
#include "GameLogger.hpp"

class Renderer
{
	private:
		static void draw_cave(Cave& cave);
		static void print_log(const GameLogger& game_logger);
	public:
		static void render(Cave& cave);
};
