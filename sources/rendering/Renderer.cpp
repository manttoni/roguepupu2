#include "rendering/Renderer.hpp"
#include "game/entities/Entity.hpp"
#include "utils/Vec2.hpp"
#include "ncurses/Color.hpp"

namespace
{
	constexpr Ncurses::Color cell_color(const Game::World::Cell& cell)
	{
		using Terrain = Game::Enum::Terrain;
		const auto& terrain = cell.terrain;

		switch(terrain)
		{
			case Terrain::Ground:
				return Ncurses::Color{1,1,1};
			case Terrain::Sand:
				return Ncurses::Color{1,2,2};
			case Terrain::Rock:
				return Ncurses::Color{2,2,2};
			case Terrain::Water:
				return Ncurses::Color{1,2,3};
			case Terrain::Grass:
				return Ncurses::Color{1,3,1};
		}
	}

	constexpr char cell_glyph(const Game::World::Cell& cell)
	{
		using Terrain = Game::Enum::Terrain;
		const auto& terrain = cell.terrain;

		switch(terrain)
		{
			case Terrain::Ground:
				return '.';
			case Terrain::Sand:
				return ',';
			case Terrain::Rock:
				return '#';
			case Terrain::Water:
				return '~';
			case Terrain::Grass:
				return '\"';
		}
	}
}

void Renderer::render(
		const Game::Simulation& simulation,
		const Game::World::GlobalPosition& center)
{
	const auto viewport_size = surface.dimensions();
	const auto top_left = center - viewport_size / 2;

	surface.clear();

	for (int y = 0; y < viewport_size.y; ++y)
	{
		for (int x = 0; x < viewport_size.x; ++x)
		{
			const auto world_position = top_left + Vec2<int>{y, x};
			const auto* cell =
				simulation.get_world().find_cell(world_position);

			if (!cell)
				continue; // Missing chunk; preparation should have loaded it.

			const auto entities =
				Game::Entity::find_all(
					simulation.get_registry(),
					Component::Value::Position{world_position},
					Component::Tag::Renderable{}
					);
			if (entities.size() == 0)
			{
				const auto color = cell_color(*cell);

				surface.enable_color(color);
				surface.put(y, x, cell_glyph(*cell));
				surface.disable_color(color);

				continue;
			}

			const auto entity = entities[frame % entities.size()]; // when multiple entities in same position, render a different one each frame
			const auto color = simulation.get_registry().get<Component::Value::Color>(entity).value;

			surface.enable_color(color);
			surface.put(y, x, simulation.get_registry().get<Component::Value::Glyph>(entity).value);
			surface.disable_color(color);
		}
	}

	surface.refresh();
	frame++;
}

void Renderer::render(const Game::Simulation& simulation)
{
	const auto player = simulation.get_player();
	const auto& position =
		simulation.get_registry()
		.get<Component::Value::Position>(player);

	render(simulation, position.value);
}
