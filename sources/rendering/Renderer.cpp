#include "rendering/Renderer.hpp"
#include "game/entities/Entity.hpp"
#include "utils/Vec2.hpp"
#include "ncurses/Color.hpp"

namespace
{
	constexpr Ncurses::Color cell_color(const Game::World::Cell& cell)
	{
		if (cell.water_depth > 0)
			return Ncurses::Color{0,0,1};

		using Material = Game::Enum::Material;

		switch(cell.material)
		{
			case Material::Stone:
				return Ncurses::Color{1,1,1};
			default:
				Log::warning() << "Unmapped cell material: " + Game::Enum::to_string<Material>(cell.material);
				return Ncurses::Color{};
		}
	}

	constexpr char cell_glyph(const Game::World::Cell& cell)
	{
		using Form = Game::Enum::Form;
		using Material = Game::Enum::Material;

		switch (cell.form)
		{
			case Form::Wall:
				return '#';
			case Form::Floor:
				if (cell.water_depth > 0)
					return '~';
				switch (cell.material)
				{
					case Material::Stone:
						return '.';
					default:
						Log::warning() << "Unmapped cell material: " + Game::Enum::to_string<Material>(cell.material);
						return '\0';
				}
			default:
				Log::warning() << "Unmapped cell form: " + Game::Enum::to_string<Form>(cell.form);
				return '?';
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
						Game::Component::Value::Position{world_position},
						Game::Component::Tag::Renderable{}
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
			const auto color = simulation.get_registry().get<Game::Component::Value::Color>(entity).value;

			surface.enable_color(color);
			surface.put(y, x, simulation.get_registry().get<Game::Component::Value::Glyph>(entity).value);
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
		.get<Game::Component::Value::Position>(player);

	render(simulation, position.value);
}
