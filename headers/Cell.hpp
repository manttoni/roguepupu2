#pragma once

#include <string>

class Cell
{
	private:
		size_t id; // also position in Area.cells vector
		std::string type;
		bool blocked;

	public:
		/* CONSTRUCTORS */
		Cell();
		Cell(const size_t id, const std::string &type);
		Cell(const Cell &other);

		/* GETTERS */
		size_t get_id() const;
		bool is_blocked() const;

		/* OVERLOADS */
		bool operator==(const Cell &other) const;
		bool operator!=(const Cell &other) const;
		bool operator<(const Cell &other) const;
		Cell &operator=(const Cell &other);
		friend std::ostream &operator<<(std::ostream &os, const Cell &cell);

};
