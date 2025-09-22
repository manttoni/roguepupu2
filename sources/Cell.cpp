#include <string>
#include <stdexcept>
#include <ostream>
#include "UI.hpp"
#include "Cell.hpp"
#include "Utils.hpp"
/* CONSTRUCTORS */
Cell::Cell()
	: idx(SIZE_MAX), type(Type::NONE), density(0) {}

Cell::Cell(const size_t idx, const Type &type, Cave* cave, const double density)
	: idx(idx), type(type), cave(cave), density(density)
{}

Cell::Cell(Cell&& other)
{
	idx = other.idx;
	type = other.type;
	density = other.density;
	fg = other.fg;
	bg = other.bg;
	entities = std::move(other.entities);
	lights = other.lights;
	cave = other.cave;
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

Cell& Cell::operator=(Cell&& other)
{
	if (this != &other)
	{
		type = other.type;
		idx = other.idx;
		density = other.density;
		fg = other.fg;
		bg = other.bg;
		entities = std::move(other.entities);
		lights = other.lights;
		cave = other.cave;
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
}

char Cell::get_char() const
{
	if (!entities.empty())
	{	// change entity each main loop to show all entities in same cell
		size_t size = entities.size();
		size_t ln = UI::instance().loop_number();
		const auto& e = entities[ln % size];
		return e->get_char();
	}
	switch (type)
	{
		case Type::ROCK:
			return static_cast<size_t>(std::ceil(density)) + '0';
		case Type::FLOOR:
			return ' ';
		case Type::SOURCE:
			return '^';
		case Type::SINK:
			return 'v';
		default:
			return '?';

	}
}

// return foreground and background colors as object
// foreground can come from entities or natural terrain
// background comes from color of natural terrain
ColorPair Cell::get_color_pair() const
{
	Color ret_fg, ret_bg;

	// foreground
	if (entities.size() > 0)
	{
		size_t ln = UI::instance().loop_number();
		const auto& e = entities[ln % entities.size()];
		ret_fg = e->get_color();
	}
	else
		ret_fg = this->fg;

	// background
	ret_bg = this->bg;

	// add light to both
	for (const auto& [light_color, light_stacks] : lights)
	{
		ret_fg += light_color * light_stacks;
		ret_bg += light_color * light_stacks;
	}

	return ColorPair(ret_fg, ret_bg);
}

bool Cell::blocks_movement() const
{
	if (type == Type::ROCK)
		return true;

	for (const auto& ent : entities)
		if (ent->blocks_movement())
			return true;

	return false;
}

bool Cell::blocks_vision() const
{
	if (type == Type::ROCK)
		return true;

	for (const auto& ent : entities)
		if (ent->blocks_vision())
			return true;

	return false;
}

