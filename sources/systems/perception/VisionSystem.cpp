#include <vector>
#include "ECS.hpp"
#include "Components.hpp"
#include "entt.hpp"
#include "systems/VisionSystem.hpp"
#include "Cell.hpp"
#include "Cave.hpp"

namespace VisionSystem
{
	double get_vision_range(const entt::registry& registry, const entt::entity entity)
	{
		if (!registry.all_of<Vision>(entity))
			return 0;
		return registry.get<Vision>(entity).range;
	}

	/* Return true if nothing blocks vision between */
	bool has_line_of_sight(const Cell& a, const Cell& b)
	{
		if (a.get_cave() != b.get_cave())
			return false;
		const Cave* cave = a.get_cave();
		const auto& cells = cave->get_cells();
		const auto start = a.get_idx();
		const auto end = b.get_idx();
		const size_t width = cave->get_width();
		int x0 = static_cast<int>(start % width);
		int y0 = static_cast<int>(start / width);
		int x1 = static_cast<int>(end % width);
		int y1 = static_cast<int>(end / width);

		int dx = abs(x1 - x0);
		int dy = abs(y1 - y0);

		int sx = x0 < x1 ? 1 : -1;
		int sy = y0 < y1 ? 1 : -1;

		int err = dx - dy;

		while (true)
		{
			size_t idx = y0 * width + x0;

			if (x0 == x1 && y0 == y1)
				break;
			if (cells[idx].blocks_vision())
				return false;
			int e2 = 2 * err;
			if (e2 > -dy)
			{
				err -= dy;
				x0 += sx;
			}
			if (e2 < dx)
			{
				err += dx;
				y0 += sy;
			}
		}

		return true;
	}

	/* Retirn true if has line of sight and is within vision range */
	bool has_vision(const entt::registry& registry, const entt::entity entity, const Cell& cell)
	{
		const Cave& cave = *cell.get_cave();
		const Cell& entity_cell = *ECS::get_cell(registry, entity);
		return has_line_of_sight(entity_cell, cell) &&
			cave.distance(entity_cell, cell) <= get_vision_range(registry, entity);
	}

	std::vector<size_t> get_visible_cells(const entt::registry& registry, const entt::entity entity)
	{
		Cell* cell = ECS::get_cell(registry, entity);
		Cave* cave = cell->get_cave();
		const auto& cells = cave->get_cells();
		const auto idx = cell->get_idx();
		const auto nearby_ids = cave->get_nearby_ids(idx, get_vision_range(registry, entity));
		std::vector<size_t> visible_cells;
		for (const auto nearby_idx : nearby_ids)
		{
			if (has_vision(registry, entity, cells[nearby_idx]))
				visible_cells.push_back(nearby_idx);
		}
		return visible_cells;
	}
};
