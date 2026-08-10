#pragma once

#include <cstddef>
#include <iosfwd>
#include <vector>

#include "domain/Cell.hpp"
#include "domain/Position.hpp"
#include "utils/Vec2.hpp"

class Cave
{
	public:
		Cave(
				size_t idx,
				size_t size,
				size_t depth,
				Cell::Type fill = Cell::Type::Rock
			);

		Cave(const Cave&) = default;
		Cave& operator=(const Cave&) = default;

		[[nodiscard]] size_t get_idx() const noexcept;
		void set_idx(size_t idx) noexcept;

		[[nodiscard]] size_t get_size() const noexcept;
		[[nodiscard]] size_t get_area() const noexcept;
		[[nodiscard]] size_t get_depth() const noexcept;

		[[nodiscard]] const std::vector<size_t>& get_connections() const noexcept;
		void set_connections(std::vector<size_t> connections);
		void add_connection(size_t connection);

		[[nodiscard]] const std::vector<Cell>& get_cells() const noexcept;
		[[nodiscard]] std::vector<Cell>& get_cells() noexcept;

		[[nodiscard]] const Cell& get_cell(size_t cell_idx) const;
		[[nodiscard]] Cell& get_cell(size_t cell_idx);

		[[nodiscard]] const Cell& get_cell(const Position& position) const;
		[[nodiscard]] Cell& get_cell(const Position& position);

		[[nodiscard]] std::vector<Position> get_positions() const;

		[[nodiscard]] std::vector<Position> get_nearby_positions(
				size_t middle,
				double radius = 1.5,
				Cell::Type type = Cell::Type::None
				) const;

		[[nodiscard]] std::vector<Position> get_nearby_positions(
				const Position& middle,
				double radius = 1.5,
				Cell::Type type = Cell::Type::None
				) const;

		[[nodiscard]] std::vector<Position> get_positions_with_type(
				Cell::Type type
				) const;

		[[nodiscard]] Position middle_position() const;
		[[nodiscard]] Position deepest_position() const;

		[[nodiscard]] double distance(
				const Position& a,
				const Position& b
				) const;

		[[nodiscard]] double distance(size_t a, size_t b) const;

		[[nodiscard]] bool contains(const Position& position) const;
		[[nodiscard]] bool contains(const Vec2<int>& vec) const;
		[[nodiscard]] bool contains(const size_t idx) const;

		[[nodiscard]] Vec2<double> get_direction(
				size_t from,
				size_t to
				) const;

		[[nodiscard]] Position offset_position(const Position& position, const Vec2<int>& offset) const;

	private:
		void validate_position(const Position& position) const;

		size_t idx{};
		size_t size{};
		size_t depth{};

		std::vector<size_t> connections;
		std::vector<Cell> cells;
};

std::ostream& operator<<(std::ostream& os, const Cave& cave);
