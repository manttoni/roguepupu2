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
	color_pair_id = other.color_pair_id;
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
		color_pair_id = other.color_pair_id;
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

void Cell::add_light(const short color_id)
{
	lights[color_id]++;
}

void Cell::reset_effects()
{
	// reset light
	lights.clear();
}

short Cell::get_color_pair_id() const
{
	short base = color_pair_id;
	size_t size = entities.size();
	if (size > 0)
	{
		size_t ln = UI::instance().loop_number();
		const auto& e = entities[ln % size];
		base = e->get_color_pair_id();
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
	short light_r = 0, light_g = 0, light_b = 0;
	for (const auto& [light_color_id, count] : lights)
	{
		Color light_color = UI::instance().get_color(light_color_id);
		light_r += light_color.get_r() * count;
		light_g += light_color.get_g() * count;
		light_b += light_color.get_b() * count;
	}
	short fg_id = UI::instance().add_color(fgr + light_r, fgg + light_g, fgb + light_b);
	short bg_id = UI::instance().add_color(bgr + light_r, bgg + light_g, bgb + light_b);
	short pair_id = UI::instance().add_color_pair(fg_id, bg_id);

	return pair_id;
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
/*
void Cell::move_entity(std::unique_ptr<Entity> entity, const Direction d)
{
	auto* cell = entity->get_cell();
	auto* cave = cell->get_cave();
	const auto& neighbor_ids = cave->get_nearby_ids(cell->get_idx(), 1.5);
	int dst = entity->get_idx();
	switch(d)
	{
		case Direction::DOWN:
			dst += cave->get_width();
			break;
		case Direction::UP:
			dst -= cave->get_width();
			break;
		case Direction::LEFT:
			dst--;
			break;
		case Direction::RIGHT:
			dst++;
			break;
	}
	if (dst < 0 || dst >= static_cast<int>(cave->get_size()))
		return;

	if (!Utils::contains(neighbor_ids, static_cast<size_t>(dst)))
		return;

	auto& new_cell = cave->get_cells()[dst];
	if (new_cell.blocks_movement())
		return;

	entity->set_cell(&new_cell);
	new_cell.add_entity(std::move(entity));
}*/
