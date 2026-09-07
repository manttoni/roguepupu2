#include <vector>
#include "rendering/Renderer.hpp"
#include "ncurses/Screen.hpp"
#include "utils/ECS.hpp"
#include "utils/Log.hpp"

Renderer::Pixel Renderer::get_pixel(const entt::registry& registry, const Domain::Position& position) const
{
	if (!ECS::get_cave(registry, position.cave_idx).contains(position))
		return Pixel{.color = Ncurses::Color{0}, .glyph = ' '};

	Pixel pixel;
	// Entities
	const auto entities = ECS::get_entities<Component::Tag::Renderable>(registry, position);
	if (!entities.empty())
	{
		const auto rendered_entity = entities[frame % entities.size()];
		pixel.glyph = registry.get<Component::Value::Glyph>(rendered_entity).value;
		pixel.color = registry.get<Ncurses::Color>(rendered_entity);
		return pixel;
	}

	// Terrain
	const auto cell_type = ECS::get_cell(registry, position).get_type();
	using Type = Domain::Cell::Type;
	switch (cell_type)
	{
		case Type::Rock:
			pixel.color = theme.rock;
			pixel.glyph = '#';
			break;
		default:
			pixel.color = theme.floor;
			pixel.glyph = '.';
			break;
	}
	return pixel;
}

void Renderer::render(const entt::registry& registry, Domain::Position center)
{
	if (!center.is_valid())
		center = ECS::get_player_position(registry);

	Log::debug() << "Rendering cave, center: " << center;

	const auto& cave = ECS::get_cave(registry, center);
	const int cave_size = static_cast<int>(cave.get_size());

	const Vec2<int> center_coords =
		Vec2<int>::from_idx(center.cell_idx, cave_size);

	auto& surface = panel.get_window();
	const Vec2<int> screen_size = surface.dimensions();

	// Vec2 uses {y, x}.
	const Vec2<int> viewport_origin{
		center_coords.y - screen_size.y / 2,
			center_coords.x - screen_size.x / 2
	};

	surface.clear();

	surface.enable_attribute(A_DIM);
	for (int screen_y = 0; screen_y < screen_size.y; ++screen_y)
	{
		for (int screen_x = 0; screen_x < screen_size.x; ++screen_x)
		{
			const Vec2<int> world{
				viewport_origin.y + screen_y,
					viewport_origin.x + screen_x
			};

			if (world.y < 0 || world.y >= cave_size ||
					world.x < 0 || world.x >= cave_size)
			{
				surface.put(screen_y, screen_x, ' ');
				continue;
			}

			const auto cell_idx = static_cast<std::size_t>(
					world.y * cave_size + world.x
					);

			const Domain::Position position{
				cell_idx,
					cave.get_idx()
			};

			const Pixel pixel = get_pixel(registry, position);

			surface.enable_color(pixel.color);
			surface.put(screen_y, screen_x, pixel.glyph);
			surface.disable_color(pixel.color);
		}
	}

	surface.refresh();
	++frame;
}

