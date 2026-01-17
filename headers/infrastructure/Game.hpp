#pragma once

#include "external/entt/entt.hpp"

class Game
{
	private:
		entt::registry registry;
		bool game_over = false;
	public:
		bool is_over() const { return game_over; }
		Game();
		void loop();
};
