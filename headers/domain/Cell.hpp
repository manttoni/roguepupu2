#pragma once

#define CELL_DENSITY_MAX 9

#include <ncurses.h>
#include <limits>
#include <cassert>
#include "ncurses/Color.hpp"  // for Ncurses::Color
#include "utils/Math.hpp"

namespace Domain
{

class Cell
{

	public:
		enum class Type
		{
			None,
			Rock,
			Floor,
			Source,
			Sink
		};
		Type get_type() const { return type; }
		void set_type(const Cell::Type type) { this->type = type; }

	private:
		Type type = Type::None;

	private:
		double density;
	public:
		void set_density(const double d) { this->density = d; }
		double get_density() const { return density; }
		double get_effective_density() const
		{
			const double limit = 1000;
			return Math::clamp(density, -limit, limit);
		}
		void reduce_density(const double amount);

		Cell(const Cell::Type type = Cell::Type::Rock);

		std::string to_string() const;

		bool operator==(const Cell &other) const { return this == &other; }
		bool operator!=(const Cell &other) const { return this != &other; }
		Cell& operator=(const Cell& other) = default;
		Cell(const Cell& other) = default;

};
} // namespace Domain
