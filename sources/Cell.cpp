#include "Cave.hpp"        // for Cave
#include "Cell.hpp"        // for Cell
#include "Color.hpp"       // for Color
#include "ColorPair.hpp"   // for ColorPair
#include "Components.hpp"  // for Renderable, Position, Opaque (ptr only)
#include "UI.hpp"          // for UI
#include "World.hpp"       // for World
#include "entt.hpp"        // for vector, basic_sigh_mixin, entity, map, size_t
#include "ECS.hpp"

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

	if (amount <= density)
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
		ret_fg = ECS::get_color(cave->get_world()->get_registry(), e);
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

	const auto& registry = cave->get_world()->get_registry();
	const size_t loop = UI::instance().get_loop_number();
	entt::entity entity = entt::null;
	const auto player = ECS::get_player(registry);
	if (ECS::can_see(registry, player, entities[0]))
		entity = entities[loop % entities_size];
	else
	{	// If player cant see this cell, it can remember only solid objects
		for (const auto e : entities)
			if (registry.all_of<Solid>(e))
				entity = e;
	}
	if (entity == entt::null)
		return L' ';
	return ECS::get_glyph(registry, entity);
}

// Use when rendering something in a single cell
// When drawing whole Cave, use Cave::draw()
void Cell::draw()
{
	auto* world = cave->get_world();
	auto& registry = world->get_registry();
	const auto& player = *registry.view<Player>().begin();
	const auto& player_position = registry.get<Position>(player);
	const size_t player_idx = player_position.cell->get_idx();

	PANEL* panel = UI::instance().get_panel(UI::Panel::GAME);
	WINDOW* window = panel_window(panel);
	UI::instance().set_current_panel(panel);

	//werase(window);
	//UI::instance().reset_colors();
	//reset_lights();

	int window_height, window_width;
	getmaxyx(window, window_height, window_width);

	const auto width = cave->get_width();
	size_t y_player = player_idx / width;
	size_t x_player = player_idx % width;

	size_t y_center = window_height / 2;
	size_t x_center = window_width / 2;

	const auto cell_idx = get_idx();
	size_t y_cell = cell_idx / width;
	size_t x_cell = cell_idx % width;

	int y = y_center + y_cell - y_player;
	int x = x_center + x_cell - x_player;
	if (y < 0 || y >= window_height || x < 0 || x >= window_width)
		return;

	ColorPair color_pair;

	if (!cave->has_vision(player_idx, cell_idx, registry.get<Vision>(player).range))
	{
		if (is_seen() && blocks_movement()) // "ghost" cell if it was seen before and was solid
			color_pair = ColorPair(Color(123, 123, 123), Color(0, 0, 0));
		else
			return;
	}
	else
		color_pair = get_color_pair();

	wchar_t glyph = get_glyph();
	UI::instance().enable_color_pair(color_pair);
	UI::instance().print_wide(y, x, glyph);
	set_seen(true);
	UI::instance().update();
}
