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
	color_pair_id = other.color_pair_id;
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
		color_pair_id = other.color_pair_id;
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

char Cell::get_char() const
{
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

void Cell::add_glow(const short glow_id)
{
	glow[glow_id]++;
}

short Cell::get_color_pair_id() const
{
	//if (glow.empty())
		return color_pair_id;
/*
	const
	const Color& fg = color_pair.get_fg();
	const Color& bg = color_pair.get_bg();
	const short fgr = fg.get_r();
	const short fgg = fg.get_g();
	const short fgb = fg.get_b();
	const short bgr = bg.get_r();
	const short bgg = bg.get_g();
	const short bgb = bg.get_b();
	const short glow_r, glow_g, glow_b;
	for (const auto& [glow_color, count] : glow)
	{
		glow_r = glow_color.get_r() * count;
		glow_g = glow_color.get_g() * count;
		glow_b = glow_color.get_b() * count;
	}
	short fg_id = add_color(fgr + glow_r, fgg + glow_g, fgb + glow_b);
	short bg_id = add_color(bgr + glow_R, bgg + glow_g, bgb + glow_b);
	short pair_id = add_color_pair(fg_id, bg_id);

	return pair_id;*/
}
