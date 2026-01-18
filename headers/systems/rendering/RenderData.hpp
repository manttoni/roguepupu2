#pragma once

#include <vector>
#include "domain/Position.hpp"

struct RenderData
{
	std::vector<Position> seen_cells;
	std::vector<Position> visible_cells;
	size_t render_frame = 0;
};
