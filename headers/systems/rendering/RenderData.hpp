#pragma once

#include <unordered_map>
#include <unordered_set>
#include <vector>

struct RenderData
{
	std::unordered_map<size_t, std::unordered_set<size_t>> seen_cells;
	std::vector<size_t> visible_cells;
	size_t render_frame = 0;
};
