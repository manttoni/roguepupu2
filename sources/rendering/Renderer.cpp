#include "rendering/Renderer.hpp"
#include "application/Layout.hpp"
#include "application/Messages.hpp"
#include "game/Query.hpp"
#include "ncurses/Screen.hpp"
#include "game/component/Runtime.hpp"
#include "game/entity/Entity.hpp"
#include "utils/Vec2.hpp"
#include "ncurses/Color.hpp"
#include "ncurses/Pixel.hpp"
#include <stdexcept>

void Renderer::redraw(
	const Game::Simulation* simulation,
	const std::optional<Game::World::GlobalPosition> cursor)
{
	auto& game = layout.get_panel(Layout::Panel::Game);
	if (!game.hidden())
	{
		auto& surface = game.get_window();
		surface.clear();
		if (simulation != nullptr)
			render_game(*simulation, cursor);
		surface.draw_border();
	}
	render_messages();
	render_player_status();
	update_panels();
	doupdate();
}

char Renderer::get_cell_glyph(const Game::World::Cell& cell)
{
	using Form = Game::Enum::Form;
	using Material = Game::Enum::Material;

	switch (cell.material)
	{
		case Material::Gravel:
			return ';';
		default:
			break;
	}
	switch (cell.form)
	{
		case Form::Wall:
			return '#';
		case Form::Floor:
			return '_';
		default:
			Log::warning()
				<< "Unmapped cell form: "
				<< Game::Enum::to_string<Form>(cell.form);
			return '?';
	}
}

Ncurses::Color Renderer::get_cell_color(const Game::World::Cell& cell)
{
	using Material = Game::Enum::Material;
	switch (cell.material)
	{
		case Material::Stone:
			if (cell.form == Game::Enum::Form::Floor)
				return Ncurses::Color{2,2,2};
			return Ncurses::Color{4,4,4};
		case Material::Soil:
			return Ncurses::Color{2,2,0};
		case Material::Gravel:
			return Ncurses::Color{4,3,3};
		default:
			Log::warning()
				<< "Unmapped cell material: "
				<< Game::Enum::to_string<Material>(cell.material);
			return Ncurses::Color{2, 0, 0};
	}
}

/* Helper to more easily get an entity at a position so that if there are
 * multiple, it chooses one based on rendering frame number.
 * */
entt::entity Renderer::get_entity(
	const Game::Simulation& simulation,
	const Game::World::GlobalPosition& position)
{
	const auto& registry = simulation.get_registry();
	const auto entities = Game::Query::find_all(
		registry,
		Game::Component::Value::Position{position},
		Game::Component::Tag::Renderable{});
	if (entities.empty())
		return entt::null;

	const auto entity = entities[frame % entities.size()];
	return entity;
}

Ncurses::Color Renderer::get_bgcolor(
	const Game::Simulation& simulation,
	const Game::World::GlobalPosition& position)
{
	Ncurses::Color bgcolor{0,0,0};

	const auto& lightmap = simulation.get_lightmap();
	if (!lightmap.contains(position))
		return bgcolor;
	const auto& position_lights = lightmap.at(position);

	for (const auto& [color_id, unused] : position_lights)
	{
		const Ncurses::Color color{color_id};
		bgcolor.r = std::max(bgcolor.r, color.r);
		bgcolor.g = std::max(bgcolor.g, color.g);
		bgcolor.b = std::max(bgcolor.b, color.b);
	}

	return bgcolor;
}

Ncurses::Pixel Renderer::get_pixel(
	const Game::Simulation& simulation,
	const Game::World::GlobalPosition& position)
{
	Ncurses::Pixel pixel;

	const auto entity = get_entity(simulation, position);
	if (entity != entt::null)
	{
		// This class touches only entities with Tag::Renderable
		// Tag::Renderable promises components for glyph and color
		// If entity doesn't have these, mistake is in earlier validation
		// Attribute is not a component, but entity can have derived attribute
		const auto& registry = simulation.get_registry();
		pixel.glyph =
			registry.get<Game::Component::Value::Glyph>(entity).value;
		pixel.fgcolor =
			registry.get<Game::Component::Value::Color>(entity).value;
		if (registry.all_of<Game::Component::Tag::Item>(entity))
			pixel.attribute |= Ncurses::Attribute{A_ITALIC};
	}

	const auto* const cell = simulation.get_grid().find_cell(position);
	if (cell == nullptr)
		throw std::runtime_error("Rendered Cell not loaded");
	if (pixel.glyph == '\0')
		pixel.glyph = get_cell_glyph(*cell);
	if (pixel.fgcolor == Ncurses::Color{})
		pixel.fgcolor = get_cell_color(*cell);
	pixel.bgcolor = get_bgcolor(simulation, position);

	pixel.fgcolor += pixel.bgcolor;

	return pixel;
}

void Renderer::render_game(
	const Game::Simulation& simulation,
	const std::optional<Game::World::GlobalPosition> cursor)
{
	auto& surface = layout.get_panel(Layout::Panel::Game).get_window();
	const auto&
		registry =
		simulation.get_registry();
	const auto
		player =
		simulation.get_player();
	const auto&
		player_position =
		registry.get<Game::Component::Value::Position>(player).value;
	const auto viewport_size = surface.content_dimensions();
	const auto offset = surface.content_offset();
	const auto
		top_left =
		player_position - viewport_size / 2;
	const auto&
		visible_positions =
		registry.get<Game::Component::Runtime::Vision>(player)
		.visible_positions;
	const auto&
		seen_positions =
		registry.get<Game::Component::Runtime::Vision>(player)
		.seen_positions;

	for (int y = 0; y < viewport_size.y; ++y)
	{
		for (int x = 0; x < viewport_size.x; ++x)
		{
			const auto position = top_left + Vec2<int>{y, x};

			const bool visible = render_all ? true :
				std::find(
					visible_positions.begin(),
					visible_positions.end(),
					position) != visible_positions.end();

			if (!visible && !seen_positions.contains(position))
				continue;

			auto pixel = get_pixel(simulation, position);

			if (!visible)
				pixel.attribute |= Ncurses::Attribute{A_DIM};

			if (cursor && position == *cursor)
				pixel.attribute |= Ncurses::Attribute{A_REVERSE};

			surface.put(y + offset.y, x + offset.x, pixel);
		}
	}

	frame++;
}

void Renderer::render_messages()
{
	auto& panel = layout.get_panel(Layout::Panel::MessageLog);
	if (panel.hidden())
		return;

	auto& surface = panel.get_window();
	surface.clear();
	const auto size = surface.content_dimensions();
	const auto offset = surface.content_offset();
	const auto height = static_cast<std::size_t>(size.y);
	const auto lines = messages.get_last(height);

	for (std::size_t i = 0; size.x > 0 && i < lines.size(); ++i)
	{
		surface.write(offset.y + static_cast<int>(height - i - 1), offset.x,
			lines[lines.size() - 1 - i]);
	}
	surface.draw_border();
}

void Renderer::render_player_status()
{
	auto& panel = layout.get_panel(Layout::Panel::PlayerStatus);
	if (panel.hidden())
		return;

	auto& surface = panel.get_window();
	surface.clear();
	// Player status contents can be drawn here when implemented.
	surface.draw_border();

	const auto& log = layout.get_panel(Layout::Panel::MessageLog);
	if (!log.hidden())
	{
		const int junction_y = log.position().y - panel.position().y;
		mvwaddch(surface.get_ptr(), junction_y, 0, ACS_RTEE);
	}
}
