#pragma once

#include "entt.hpp"

class Game
{
	private:
		entt::registry registry;
		entt::entity player;
		bool game_over = false;
	public:
		bool is_over() const { return game_over; }
		Game();
		void loop();
};
