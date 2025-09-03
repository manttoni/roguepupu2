#include "Cell.hpp"
#include "Utils.hpp"
#include <string>
#include <stdexcept>
#include <ostream>

/* CONSTRUCTORS */
Cell::Cell() : id(SIZE_MAX), type("unknown"), blocked(true) {}

Cell::Cell(const size_t id, const std::string &type, const int rock_thickness) : id(id), type(type), rock_thickness(rock_thickness)
{
	if (type == "rock")
		blocked = true;
	else if (type == "floor")
		blocked = false;
	else if (type == "stream_source")
		blocked = false;
	else if (type == "stream_sink")
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
	if (cell.type == "floor")
		os << 'f';
	else if (cell.type == "rock")
		os << cell.rock_thickness;
	else if (cell.type == "stream_source")
		os << '^';
	else if (cell.type == "stream_sink")
		os << 'v';
	else
		throw std::runtime_error("Cell operator<< check types");
	return os;
}

void Cell::erode(const int amount)
{
	if (type != "rock")
		return;
	if (rock_thickness > 0)
		rock_thickness -= Random::randint(amount - 1, amount);
	if (rock_thickness > 0)
		return;
	rock_thickness = 0;
	type = "floor";
	blocked = false;
}
