#pragma once

#include <string>

class Cell
{
	private:
		size_t id;
		std::string type;
		bool blocked;
		double density = 0;

	public:
		/* CONSTRUCTORS */
		Cell();
		Cell(const size_t id, const std::string& type, const double density = 0);
		Cell(const Cell &other);

		/* GETTERS */
		size_t get_id() const { return id; }
		bool is_blocked() const { return blocked; }
		std::string get_type() const { return type; }
		double get_density() const { return density; }

		/* OVERLOADS */
		bool operator==(const Cell &other) const;
		bool operator!=(const Cell &other) const;
		bool operator<(const Cell &other) const;
		Cell &operator=(const Cell &other);
		friend std::ostream &operator<<(std::ostream &os, const Cell &cell);

		/* MODIFY CELL */
		void reduce_density(const double amount);

};
