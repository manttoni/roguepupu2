#include "Cell.hpp"
#include "Utils.hpp"
#include <string>
#include <stdexcept>
#include <ostream>

/* CONSTRUCTORS */
Cell::Cell() : id(SIZE_MAX), type("unknown"), blocked(true) {}

Cell::Cell(const size_t id, const std::string &type) : id(id), type(type)
{
	if (type == "wall")
	{
		rock_thickness = Random::randint(1, 10);
		blocked = true;
	}
	else if (type == "floor")
		blocked = false;
	else
		throw std::runtime_error("Check cell types");
}

Cell::Cell(const Cell &other)
{
	id = other.id;
	type = other.type;
	blocked = other.blocked;
	rock_thickness = other.rock_thickness;
}

/* OVERLOADS */

bool Cell::operator==(const Cell &other) const
{
	return id == other.id;
}

bool Cell::operator!=(const Cell &other) const
{
	return id != other.id;
}

bool Cell::operator<(const Cell &other) const
{
	return id < other.id;
}

Cell &Cell::operator=(const Cell &other)
{
	if (this != &other)
	{
		type = other.type;
		blocked = other.blocked;
		id = other.id;
		rock_thickness = other.rock_thickness;
	}
	return *this;
}

std::ostream &operator<<(std::ostream &os, const Cell &cell)
{
	if (cell.type == "wall") assert(cell.rock_thickness > 0);
	os << cell.type.front() << Colors::RESET;
	return os;
}

