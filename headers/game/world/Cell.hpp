#pragma once

#include "game/Enum.hpp"

namespace Game::World
{
	struct Cell
	{
		Enum::Material material;
		Enum::Form form;
		double water_depth;
	};
}
