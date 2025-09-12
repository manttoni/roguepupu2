#pragma once

#include <string>

class Cell
{
	public:
		enum class Type
		{
			ROCK,	// solid rock
			FLOOR,	// empty space with a floor
			SOURCE,	// previous level
			SINK,	// next level
			NONE,	// can be anything
		};

	private:
		size_t idx;
		Type type;
		bool blocked;
		double density;

	public:
		/* CONSTRUCTORS */
		Cell();
		Cell(const size_t idx, const Type& type, const double density = 0);
		Cell(const Cell &other);

		/* GETTERS */
		size_t get_idx() const { return idx; }
		bool is_blocked() const { return blocked; }
		Type get_type() const { return type; }
		double get_density() const { return density; }

		/* SETTERS */
		void set_type(const Cell::Type type) { this->type = type; }
		void set_blocked(const bool b) { this->blocked = b; }
		void set_density(const double d) { this->density = d; }

		/* OVERLOADS */
		bool operator==(const Cell &other) const;
		bool operator!=(const Cell &other) const;
		bool operator<(const Cell &other) const;
		Cell &operator=(const Cell &other);

		/* MODIFY CELL */
		void reduce_density(const double amount);
};
