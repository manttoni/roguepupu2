#pragma once
#include <nlohmann/json.hpp>

#include <sstream>
#include "utils/Vec2.hpp"

namespace Game::World
{
	template <typename T>
		struct Position
		{
			int y;
			int x;

			Vec2<int> vec2() const { return Vec2<int>{y, x}; }

			bool operator==(const Position&) const = default;
			Position& operator+=(const Vec2<int>& offset)
			{
				y += offset.y;
				x += offset.x;
				return *this;
			}

			Position& operator-=(const Vec2<int>& offset)
			{
				y -= offset.y;
				x -= offset.x;
				return *this;
			}

			Position operator+(const Vec2<int>& offset) const
			{
				auto result = *this;
				result += offset;
				return result;
			}

			Position operator-(const Vec2<int>& offset) const
			{
				auto result = *this;
				result -= offset;
				return result;
			}

			// Difference between positions in the same coordinate space.
			Vec2<int> operator-(const Position& other) const
			{
				return {y - other.y, x - other.x};
			}

		};

	struct ChunkSpace {};
	struct GlobalSpace {};
	struct LocalSpace {};

	using ChunkPosition = Position<ChunkSpace>;
	using GlobalPosition = Position<GlobalSpace>;
	using LocalPosition = Position<LocalSpace>;

	ChunkPosition to_chunk(const GlobalPosition& global);
	GlobalPosition to_global(const ChunkPosition& chunk, const LocalPosition& local);
	LocalPosition to_local(const ChunkPosition& chunk, const GlobalPosition& global);

	template <typename T>
		std::ostream& operator<<(std::ostream& out, const Position<T>& position)
		{
			return out << '(' << position.y << ", " << position.x << ')';
		}

	template <typename T>
		std::array<Position<T>, 8> neighbors(const Position<T>& position)
		{
			return {{
				{position.y - 1, position.x - 1},
					{position.y - 1, position.x    },
					{position.y - 1, position.x + 1},
					{position.y,     position.x - 1},
					{position.y,     position.x + 1},
					{position.y + 1, position.x - 1},
					{position.y + 1, position.x    },
					{position.y + 1, position.x + 1}
			}};
		}

	template <typename T>
		void from_json(const nlohmann::json& j, Position<T>& position)
		{
			position.y = j.at("y").get<int>();
			position.x = j.at("x").get<int>();
		}

	template <typename T>
		void to_json(nlohmann::json& j, const Position<T>& position)
		{
			j = {
				{"y", position.y},
				{"x", position.x}
			};
		}
}

