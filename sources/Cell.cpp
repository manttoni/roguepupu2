#include <curses.h>        // for WINDOW, getmaxyx
#include <panel.h>         // for panel_window, PANEL
#include "Cave.hpp"        // for Cave
#include "Cell.hpp"        // for Cell
#include "Color.hpp"       // for Color
#include "ColorPair.hpp"   // for ColorPair
#include "Components.hpp"  // for Position, Vision, Solid (ptr only), Opaque...
#include "ECS.hpp"         // for can_see, get_color, get_glyph, get_player
#include "UI.hpp"          // for UI
#include "World.hpp"       // for World
#include "entt.hpp"        // for vector, basic_sigh_mixin, size_t, entity
#include "GameState.hpp"
#include "Unicode.hpp"

Cell::Cell() : idx(SIZE_MAX), type(Type::None), density(0) {}

Cell::Cell(const size_t idx, const Type &type, Cave* cave, const wchar_t glyph, const double density)
	: idx(idx), type(type), cave(cave), glyph(glyph), density(density)
{}

bool Cell::operator==(const Cell &other) const
{
	return idx == other.idx && cave == other.cave;
}

bool Cell::operator!=(const Cell &other) const
{
	return !operator==(other);
}

bool Cell::operator<(const Cell &other) const
{
	return idx < other.idx;
}

bool Cell::is_border() const
{
	const size_t width = cave->get_width();
	const size_t height = cave->get_height();
	const size_t y = idx / width;
	const size_t x = idx % width;
	return y == 0 || y == height - 1 || x == 0 || x == width - 1;
}

void Cell::reduce_density(const double amount)
{
	density -= amount;
	if (density > 0 || type == Type::Floor || is_border())
		return;
	type = Type::Floor;
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

bool Cell::is_empty() const
{
	if (!get_entities().empty())
		return false;
	return type == Type::Floor;
}

bool Cell::has_landmark() const
{
	if (type == Type::Rock)
		return true;
	auto* world = cave->get_world();
	auto& registry = world->get_registry();
	const auto& entities = get_entities();
	for (const auto& e : entities)
	{
		if (registry.all_of<Landmark>(e))
			return true;
	}
	return false;
}

bool Cell::blocks_movement() const
{
	if (type == Type::Rock)
		return true;

	for (const auto& ent : get_entities())
		if (cave->get_world()->get_registry().all_of<Solid>(ent))
			return true;

	return false;
}

bool Cell::blocks_vision() const
{
	if (type == Type::Rock)
		return true;

	for (const auto& ent : get_entities())
		if (cave->get_world()->get_registry().all_of<Opaque>(ent))
			return true;

	return false;
}


