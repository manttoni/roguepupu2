#pragma once

#include <array>
#include <cstddef>

#include "game/world/Cell.hpp"
#include "utils/Vec2.hpp"

namespace Game::World
{
	using LocalPosition = Vec2<int>;

	struct SpawnRequest
	{
		LocalPosition position;
		std::string definition_id;
	};

	class Chunk
	{
		public:
			static constexpr int width = 100;
			static constexpr int height = 100;
			static inline Vec2<int> dimensions() { return Vec2<int>{height, width}; }

			const Cell& get_cell(const LocalPosition& position) const;
			Cell& get_cell(const LocalPosition& position);

		private:
			std::array<Cell, width * height> cells;
			std::vector<SpawnRequest> spawn_requests;

			static constexpr std::size_t to_index(const LocalPosition& position)
			{
				return position.y * width + position.x;
			}
	};
}
