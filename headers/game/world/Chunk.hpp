#pragma once

#include <array>
#include <cstddef>

#include "game/world/Cell.hpp"
#include "game/world/Position.hpp"
#include "utils/Vec2.hpp"

namespace Game::World
{
	class Chunk
	{
		public:
			static constexpr int width = 100;
			static constexpr int height = 100;
			static inline Vec2<int> dimensions() { return Vec2<int>{height, width}; }

			using CellsArray = std::array<Cell, width * height>;

			const Cell& get_cell(const LocalPosition& position) const;
			Cell& get_cell(const LocalPosition& position);

		private:
			CellsArray cells;

			static constexpr std::size_t to_index(const LocalPosition& position)
			{
				return position.y * width + position.x;
			}
	};
}
