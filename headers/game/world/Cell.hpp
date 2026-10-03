#pragma once

#include "game/Enum.hpp"

namespace Game::World
{
	struct Cell
	{
		Enum::Material material = Enum::Material::None;
		Enum::Form form = Enum::Form::None;
		double water_depth{};
	};
}
