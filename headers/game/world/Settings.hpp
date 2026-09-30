#pragma once

#include <cstddef>
#include "utils/Vec2.hpp"

namespace Game::World
{
	struct Settings
	{
		Vec2<int> preload_extent{500, 500};
	};
}
