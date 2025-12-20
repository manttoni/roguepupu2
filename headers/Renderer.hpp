#pragma once

#include "Cave.hpp"
#include "GameLogger.hpp"
#include "entt.hpp"

class Renderer
{
	private:
		static void draw_cave(Cave& cave);
		static void print_log(const GameLogger& game_logger);
		static void show_status(const entt::registry& registry);
		static void draw_bar(const Color& color, const double percentage, const size_t y, const size_t x);
	public:
		static void render(Cave& cave);
};
