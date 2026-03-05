#pragma once

#include "external/entt/entt.hpp"

class Cave;

class Game
{
	private:
		entt::registry registry;
		bool game_over = false;
	public:
		bool is_over() const { return game_over; }
		Game();
		void loop();
		entt::entity select_character(entt::registry& registry);
		Cave& create_spawn_room(entt::registry& registry);
};
