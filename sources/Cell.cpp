#include "Cell.hpp"
#include "Utils.hpp"
#include <string>
#include <stdexcept>
#include <ostream>

/* CONSTRUCTORS */
Cell::Cell()
	: idx(SIZE_MAX), type(Type::NONE), blocked(true), density(0) {}

Cell::Cell(const size_t idx, const Type &type, const double density)
	: idx(idx), type(type), density(density)
{
	if (type == Type::ROCK)
		blocked = true;
	else if (type == Type::FLOOR)
		blocked = false;
	else if (type == Type::SOURCE)
		blocked = false;
	else if (type == Type::SINK)
		blocked = false;
	else
		throw std::runtime_error("Check cell types");
}

Cell::Cell(const Cell &other)
{
	idx = other.idx;
	type = other.type;
	blocked = other.blocked;
	density = other.density;
}

/* OVERLOADS */
bool Cell::operator==(const Cell &other) const
{
	return idx == other.idx;
}

bool Cell::operator!=(const Cell &other) const
{
	return idx != other.idx;
}

bool Cell::operator<(const Cell &other) const
{
	return idx < other.idx;
}

Cell &Cell::operator=(const Cell &other)
{
	if (this != &other)
	{
		type = other.type;
		blocked = other.blocked;
		idx = other.idx;
		density = other.density;
	}
	return *this;
}

void Cell::reduce_density(const double amount)
{
	if (type != Type::ROCK)
		return; // for now only rock can weaken

	if (amount < density)
	{
		density -= amount;
		return;
	}

	// rock has been destroyed
	density = 0;
	type = Type::FLOOR;
	blocked = false;
}
