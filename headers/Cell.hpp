#pragma once

#include <string>

class Cell
{
	private:
		size_t id; // also position in Area.cells vector
		std::string type;
		bool blocked;
		int rock_thickness = 0;

	public:
		/* CONSTRUCTORS */
		Cell();
		Cell(const size_t id, const std::string &type, const int rock_thickness = 0);
		Cell(const Cell &other);

		/* GETTERS */
		size_t get_id() const { return id; }
		bool is_blocked() const { return blocked; }
		std::string get_type() const { return type; }
		int get_rock_thickness() const { return rock_thickness; }

		/* OVERLOADS */
		bool operator==(const Cell &other) const;
		bool operator!=(const Cell &other) const;
		bool operator<(const Cell &other) const;
		Cell &operator=(const Cell &other);
		friend std::ostream &operator<<(std::ostream &os, const Cell &cell);

		/* MODIFY CELL */
		void erode(const int amount);

};
