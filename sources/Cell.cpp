#include "Cell.hpp"
#include "Utils.hpp"
#include <string>
#include <stdexcept>
#include <ostream>

/* CONSTRUCTORS */
Cell::Cell() : id(SIZE_MAX), type("unknown"), blocked(true) {}

Cell::Cell(const size_t id, const std::string &type, const double density) : id(id), type(type), density(density)
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
	density = other.density;
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
		density = other.density;
	}
	return *this;
}

std::ostream &operator<<(std::ostream &os, const Cell &cell)
{
	if (cell.type == "floor")
		os << 'f';
	else if (cell.type == "rock")
		os << cell.density;
	else if (cell.type == "stream_source")
		os << '^';
	else if (cell.type == "stream_sink")
		os << 'v';
	else
		throw std::runtime_error("Cell operator<< check types");
	return os;
}

void Cell::reduce_density(const double amount)
{
	if (type != "rock")
		return; // for now only rock can weaken

	if (amount < density)
	{
		density -= amount;
		return;
	}
	// rock has been destroyed
	density = 0;

	type = "floor";
	blocked = false;
}
