#include "rendering/Renderer.hpp"
#include "game/component/Runtime.hpp"
#include "game/entity/Entity.hpp"
#include "utils/Vec2.hpp"
#include "ncurses/Color.hpp"
#include <stdexcept>

Renderer::Visual Renderer::get_cell_visual(const Game::World::Cell& cell)
{
	using Form = Game::Enum::Form;
	using Material = Game::Enum::Material;

	Renderer::Visual visual;

	switch (cell.form)
	{
		case Form::Wall:
			visual.glyph = '#';
			break;
		case Form::Floor:
			visual.glyph = '_';
			break;
		default:
			Log::warning()
				<< "Unmapped cell form: "
				<< Game::Enum::to_string<Form>(cell.form);
			visual.glyph = '?';
	}

	switch (cell.material)
	{
		case Material::Stone:
			visual.color = Ncurses::Color{1,1,1};
			break;
		case Material::Soil:
			visual.color = Ncurses::Color{1,1,0};
			break;
		default:
			Log::warning()
				<< "Unmapped cell material: "
				<< Game::Enum::to_string<Material>(cell.material);
			visual.color = Ncurses::Color{1, 0, 0};
	}
	return visual;
}

std::optional<Renderer::Visual> Renderer::get_entity_visual(
		const Game::Simulation& simulation,
		const Game::World::GlobalPosition& position)
{
	const auto& registry = simulation.get_registry();
	const auto entities = Game::Entity::find_all(
			registry,
			Game::Component::Value::Position{position});
	if (entities.empty())
		return std::nullopt;

	const auto entity = entities[frame % entities.size()];

	Renderer::Visual visual;
	visual.glyph = registry.get<Game::Component::Value::Glyph>(entity).value;
	visual.color = registry.get<Game::Component::Value::Color>(entity).value;
	return visual;
}

Renderer::Visual Renderer::get_visual(
		const Game::Simulation& simulation,
		const Game::World::GlobalPosition& position)
{
	const auto entity_visual =
		get_entity_visual(simulation, position);
	if (entity_visual.has_value())
		return entity_visual.value();

	auto* cell = simulation.get_grid().find_cell(position);
	if (cell == nullptr)
		throw std::runtime_error(
				std::string("Cell not loaded at: ")
				+ position.to_string());
	return get_cell_visual(*cell);
}

void Renderer::render_player_perspective(const Game::Simulation& simulation)
{
	const auto& registry = simulation.get_registry();
	const auto player = simulation.get_player();
	if (!registry.all_of<Game::Component::Runtime::Vision>(player))
		throw std::runtime_error("Player has no Vision component");
	const auto& player_position =
		registry.get<Game::Component::Value::Position>(player).value;
	const auto viewport_size = surface.dimensions();
	const auto top_left = player_position - viewport_size / 2;
	const auto& visible_positions =
		registry.get<Game::Component::Runtime::Vision>(player).visible;
	const auto& seen_positions =
		registry.get<Game::Component::Runtime::Vision>(player).seen;

	surface.clear();

	for (int y = 0; y < viewport_size.y; ++y)
	{
		for (int x = 0; x < viewport_size.x; ++x)
		{
			const auto position = top_left + Vec2<int>{y, x};

			if (
					std::find(
						visible_positions.begin(),
						visible_positions.end(),
						position) != visible_positions.end())
			{
				const auto visual = get_visual(simulation, position);
				surface.put(y, x, visual.glyph, visual.color);
			}
			else if (
					std::find(
						seen_positions.begin(),
						seen_positions.end(),
						position) != seen_positions.end())
			{
				const auto visual = get_visual(simulation, position);
				surface.enable_attribute(Ncurses::Attribute{A_DIM});
				surface.put(y, x, visual.glyph, visual.color);
				surface.disable_attribute(Ncurses::Attribute{A_DIM});
			}
		}
	}

	surface.refresh();
	frame++;
}


