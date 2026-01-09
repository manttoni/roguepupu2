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
	if (is_border()) return;
	density -= amount;
	if (density > 0 || type == Type::Floor)
		return;
	type = Type::Floor;
	glyph = L' ';
	bgcolor = Color(35, 40, 30);
}

void Cell::add_liquid(const Liquid::Type liquid, const double amount)
{
	assert(amount >= 0);
	liquids[liquid] += amount;
}

double Cell::remove_liquid(const Liquid::Type liquid, const double amount)
{
	assert(amount >= 0);
	const double removed = std::max(amount, liquids[liquid]);
	liquids[liquid] -= removed;
	if (liquids[liquid] <= 0)
		liquids.erase(liquid);
	return removed;
}

double Cell::get_liquids_amount() const
{
	double liquids_amount = 0;
	for (const auto& [type, amount] : liquids)
		liquids_amount += amount;
	return liquids_amount;
}

// how high is waterlevel
double Cell::get_liquid_level() const
{
	double liquid_level = density;
	for (const auto& [type, amount] : liquids)
		liquid_level += amount;
	return liquid_level;
}

Color Cell::get_fgcolor() const
{
	if (liquids.empty())
		return fgcolor;
	return Color(0, 100, 300);
}

Color Cell::get_bgcolor() const
{
	if (liquids.empty())
		return bgcolor;
	return Color(0, 10, 30);
}

wchar_t Cell::get_glyph() const
{
	if (glyph - L'0' >= 0 && glyph - L'9' < 10) // DevTools thing
		return glyph;
	if (liquids.empty())
		return glyph;
	const double liquids_amount = get_liquids_amount();
	if (liquids_amount < 0.1)
		return Unicode::LiquidShallow;
	if (liquids_amount < 0.5)
		return Unicode::LiquidMedium;
	return Unicode::LiquidDeep;
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


