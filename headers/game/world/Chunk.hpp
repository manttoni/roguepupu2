#pragma once

#include <ostream>
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
			static constexpr int width = 50;
			static constexpr int height = 50;
			static inline Vec2<int> dimensions()
			{
				return Vec2<int>{height, width};
			}

			Chunk() = default;
			Chunk(	Game::Enum::Material material,
					Game::Enum::Form form);

			using CellsArray = std::array<Cell, width * height>;

			const Cell& get_cell(const LocalPosition& position) const;
			Cell& get_cell(const LocalPosition& position);

			const CellsArray& get_cells() const { return cells; }

		private:
			CellsArray cells;

			static constexpr std::size_t to_index(const LocalPosition& position)
			{
				return position.y * width + position.x;
			}
	};

	inline std::ostream& operator<<(std::ostream& os, const Chunk& chunk)
	{
		size_t walls = 0;
		size_t floors = 0;
		for (const auto& cell : chunk.get_cells())
		{
			if (cell.form == Enum::Form::Wall)
				walls++;
			else if (cell.form == Enum::Form::Floor)
				floors++;
		}
		os << "{ walls: " << walls << ", floors: " << floors << " }";
		return os;
	}
}
