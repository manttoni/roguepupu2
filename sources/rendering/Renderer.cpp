#include "rendering/Renderer.hpp"
#include "ncurses/Screen.hpp"
#include "game/component/Runtime.hpp"
#include "game/entity/Entity.hpp"
#include "utils/Vec2.hpp"
#include "ncurses/Color.hpp"
#include "ncurses/Pixel.hpp"
#include <stdexcept>

char Renderer::get_cell_glyph(const Game::World::Cell& cell)
{
	using Form = Game::Enum::Form;

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
			return Ncurses::Color{1,1,1};
		case Material::Soil:
			return Ncurses::Color{1,1,0};
		default:
			Log::warning()
				<< "Unmapped cell material: "
				<< Game::Enum::to_string<Material>(cell.material);
			return Ncurses::Color{1, 0, 0};
	}
}

entt::entity Renderer::get_entity(
		const Game::Simulation& simulation,
		const Game::World::GlobalPosition& position)
{
	const auto& registry = simulation.get_registry();
	const auto entities = Game::Entity::find_all(
			registry,
			Game::Component::Value::Position{position},
			Game::Component::Tag::Renderable{});
	if (entities.empty())
		return entt::null;

	const auto entity = entities[frame % entities.size()];
	return entity;
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
		// Give some attribute based on some Tag for example, TODO
	}

	const auto* const cell = simulation.get_grid().find_cell(position);
	if (cell == nullptr)
		throw std::runtime_error("Rendered Cell not loaded");
	if (pixel.glyph == '\0')
		pixel.glyph = get_cell_glyph(*cell);
	if (pixel.fgcolor == Ncurses::Color{})
		pixel.fgcolor = get_cell_color(*cell);
	// pixel.bgcolor = lightmap

	return pixel;
}

void Renderer::render_player_perspective(
		const Game::Simulation& simulation,
		const std::optional<Game::World::GlobalPosition> cursor)
{
	const auto&
		registry =
		simulation.get_registry();
	const auto
		player =
		simulation.get_player();
	const auto&
		player_position =
		registry.get<Game::Component::Value::Position>(player).value;
	const auto
		viewport_size =
		surface.dimensions();
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

	surface.clear();

	for (int y = 0; y < viewport_size.y; ++y)
	{
		for (int x = 0; x < viewport_size.x; ++x)
		{
			const auto position = top_left + Vec2<int>{y, x};

			const bool visible =
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

			surface.put(y, x, pixel);
		}
	}

	surface.refresh();
	frame++;
}

void Renderer::write(const std::vector<std::string>& messages)
{
	const auto height =
		static_cast<std::size_t>(Ncurses::Screen::size().y);
	const auto messages_size = messages.size();

	for (std::size_t i = 0; i < messages_size && i < height; ++i)
	{
		surface.write(height - i - 1, 0, messages[messages_size - 1 - i]);
	}
	surface.refresh();
}
