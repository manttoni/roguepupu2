#pragma once

#include <nlohmann/json.hpp>
#include <cstddef>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <vector>
#include <functional>
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

			std::string to_string() const
			{
				return
					"(" + std::to_string(y)
					+ ", "
					+ std::to_string(x) + ")";
			}
		};

	struct ChunkSpace {};
	struct GlobalSpace {};
	struct LocalSpace {};

	using ChunkPosition = Position<ChunkSpace>;
	using GlobalPosition = Position<GlobalSpace>;
	using LocalPosition = Position<LocalSpace>;

	ChunkPosition to_chunk(const GlobalPosition& global);
	GlobalPosition to_global(
			const ChunkPosition& chunk,
			const LocalPosition& local);
	LocalPosition to_local(
			const ChunkPosition& chunk,
			const GlobalPosition& global);

	template <typename T>
		std::ostream& operator<<(std::ostream& out, const Position<T>& position)
		{
			return out << position.to_string();
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
		double distance(const Position<T>& a, const Position<T>& b)
		{
			return std::hypot(
					static_cast<double>(a.y) - static_cast<double>(b.y),
					static_cast<double>(a.x) - static_cast<double>(b.x));
		}

	template <typename T>
		std::vector<Position<T>> nearby(
				const Position<T>& position,
				const double radius)
		{
			std::vector<Position<T>> positions;

			if (!std::isfinite(radius) || radius < 0.0)
				return positions;

			// Keep the search bounds within representable coordinates.
			const auto bounded = [](double coordinate)
			{
				return static_cast<std::int64_t>(std::clamp(
							coordinate,
							static_cast<double>(std::numeric_limits<int>::min()),
							static_cast<double>(std::numeric_limits<int>::max())));
			};

			const auto min_y = bounded(std::ceil(position.y - radius));
			const auto max_y = bounded(std::floor(position.y + radius));
			const auto min_x = bounded(std::ceil(position.x - radius));
			const auto max_x = bounded(std::floor(position.x + radius));
			const auto radius_squared = radius * radius;

			for (auto y = min_y; y <= max_y; ++y)
			{
				const double dy = static_cast<double>(y) - position.y;

				for (auto x = min_x; x <= max_x; ++x)
				{
					const double dx = static_cast<double>(x) - position.x;

					if (dx * dx + dy * dy <= radius_squared)
					{
						positions.push_back({
								static_cast<int>(y),
								static_cast<int>(x)});
					}
				}
			}

			std::sort(positions.begin(), positions.end(),
				[&position](auto& a, auto& b)
				{
				return distance(a, position) < distance(b, position);
				});

			return positions;
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

namespace std
{
	template <typename Space>
		struct hash<Game::World::Position<Space>>
		{
			std::size_t operator()(
					const Game::World::Position<Space>& position) const noexcept
			{
				const auto hy = std::hash<int>{}(position.y);
				const auto hx = std::hash<int>{}(position.x);

				return hy ^ (hx + std::size_t{0x9e3779b9}
						+ (hy << 6) + (hy >> 2));
			}
		};
}
