#include "Cave.hpp"        // for Cave
#include "Cell.hpp"        // for Cell
#include "Color.hpp"       // for Color
#include "ColorPair.hpp"   // for ColorPair
#include "Components.hpp"  // for Renderable, Position, Opaque (ptr only)
#include "UI.hpp"          // for UI
#include "World.hpp"       // for World
#include "entt.hpp"        // for vector, basic_sigh_mixin, entity, map, size_t

Cell::Cell() : idx(SIZE_MAX), type(Type::NONE), density(0) {}

Cell::Cell(const size_t idx, const Type &type, Cave* cave, const wchar_t glyph, const double density)
	: idx(idx), type(type), cave(cave), glyph(glyph), density(density)
{}

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
	glyph = L' ';
	bg = Color(35, 40, 30);
}

std::vector<entt::entity> Cell::get_entities() const
{
	std::vector<entt::entity> entities;
	auto& registry = cave->get_world()->get_registry();
	registry.view<Position>().each([this, &entities](auto entity, auto& pos)
			{
				if (pos.cell == this)
					entities.push_back(entity);
			});
	return entities;
}

// return foreground and background colors as object
// foreground can come from entities or natural terrain
// background comes from color of natural terrain
ColorPair Cell::get_color_pair() const
{
	Color ret_fg, ret_bg;
	const auto& entities = get_entities();

	// foreground
	if (entities.size() > 0)
	{
		size_t ln = UI::instance().get_loop_number();
		const auto& e = entities[ln % entities.size()];
		ret_fg = cave->get_world()->get_registry().get<Renderable>(e).color;
	}
	else
		ret_fg = this->fg;

	// background
	ret_bg = this->bg;

	// add light to both
	for (const auto& [light_color, light_stacks] : lights)
	{
		ret_fg += light_color * static_cast<int>(light_stacks);
		ret_bg += light_color * static_cast<int>(light_stacks);
	}

	return ColorPair(ret_fg, ret_bg);
}

bool Cell::blocks_movement() const
{
	if (type == Type::ROCK)
		return true;

	for (const auto& ent : get_entities())
		if (cave->get_world()->get_registry().all_of<Solid>(ent))
			return true;

	return false;
}

bool Cell::blocks_vision() const
{
	if (type == Type::ROCK)
		return true;

	for (const auto& ent : get_entities())
		if (cave->get_world()->get_registry().all_of<Opaque>(ent))
			return true;

	return false;
}

wchar_t Cell::get_glyph() const
{
	const auto& entities = get_entities();
	size_t entities_size = entities.size();
	if (entities_size == 0)
		return glyph;

	const auto& entity = entities[UI::instance().get_loop_number() % entities_size];
	return cave->get_world()->get_registry().get<Renderable>(entity).glyph;
}
