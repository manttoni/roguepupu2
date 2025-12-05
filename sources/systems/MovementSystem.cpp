#include <map>
#include "entt.hpp"
#include "Utils.hpp"
#include "Components.hpp"
#include "Cave.hpp"
#include "Cell.hpp"
#include "systems/MovementSystem.hpp"

namespace MovementSystem
{
	std::map<int, Vec2> movement_keys = {
		{KEY_UP,	{-1, 0}},
		{KEY_RIGHT,	{0, 1}},
		{KEY_DOWN,	{1, 0}},
		{KEY_LEFT,	{0, -1}}
	};

	double move(entt::registry& reg, entt::entity e, const Vec2 d)
	{
		auto& pos = reg.get<Position>(e);
		auto* current_cave = pos.cell->get_cave();
		const size_t width = current_cave->get_width();

		const size_t src_idx = pos.cell->get_idx();
		const size_t y = src_idx / width;
		const size_t x = src_idx % width;

		const size_t dst_idx = (d.dy + y) * width + d.dx + x;
		if (!current_cave->has_access(src_idx, dst_idx))
			return 0;
		pos.cell = &current_cave->get_cells()[dst_idx];
		return current_cave->distance(src_idx, dst_idx);
	}
};
