#include <string>
#include <stdexcept>
#include <ostream>
#include "UI.hpp"
#include "Cell.hpp"
#include "Utils.hpp"
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
	entities = other.entities;
	glow = other.glow;
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
		entities = other.entities;
		glow = other.glow;
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
	if (!entities.empty())
	{	// change entity each main loop to show all entities in same cell
		size_t size = entities.size();
		size_t ln = UI::instance().loop_number();
		Entity e = entities[ln % size];
		return e.get_char();
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

void Cell::add_glow(const short glow_id)
{
	glow[glow_id]++;
}

void Cell::reset_effects()
{
	// reset glow
	glow.clear();
}

short Cell::get_color_pair_id() const
{
	short base = color_pair_id;
	size_t size = entities.size();
	if (size > 0)
	{
		size_t ln = UI::instance().loop_number();
		Entity e = entities[ln % size];
		base = e.get_color_pair_id();
	}

	ColorPair cp = UI::instance().get_color_pair(base);
	const Color& fg = cp.get_fg();
	const Color& bg = cp.get_bg();
	const short fgr = fg.get_r();
	const short fgg = fg.get_g();
	const short fgb = fg.get_b();
	const short bgr = bg.get_r();
	const short bgg = bg.get_g();
	const short bgb = bg.get_b();
	short glow_r = 0, glow_g = 0, glow_b = 0;
	for (const auto& [glow_color_id, count] : glow)
	{
		Color glow_color = UI::instance().get_color(glow_color_id);
		glow_r += glow_color.get_r() * count;
		glow_g += glow_color.get_g() * count;
		glow_b += glow_color.get_b() * count;
	}
	short fg_id = UI::instance().add_color(fgr + glow_r, fgg + glow_g, fgb + glow_b);
	short bg_id = UI::instance().add_color(bgr + glow_r, bgg + glow_g, bgb + glow_b);
	short pair_id = UI::instance().add_color_pair(fg_id, bg_id);

	return pair_id;
}
