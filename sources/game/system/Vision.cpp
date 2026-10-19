#include "game/system/Vision.hpp"
#include "game/component/Runtime.hpp"
#include "game/entity/Entity.hpp"

#include <cstdint>
#include <cstdlib>

namespace Game::System::Vision
{
	bool los(
			const entt::registry& registry,
			const World::Grid& grid,
			World::GlobalPosition a,
			World::GlobalPosition b)
	{
		// Both endpoints must exist, but may be opaque.
		if (!grid.find_cell(a) || !grid.find_cell(b))
			return false;

		const auto blocked = [&](World::GlobalPosition position)
		{
			const auto* const cell = grid.find_cell(position);

			if (cell == nullptr || cell->form == Enum::Form::Wall)
				return true;

			return !Entity::find_all(
					registry,
					Component::Value::Position{position},
					Component::Value::CollisionVision{true}
					).empty();
		};

		const int step_x = (b.x > a.x) - (b.x < a.x);
		const int step_y = (b.y > a.y) - (b.y < a.y);

		// Cast before subtraction to avoid overflowing an int.
		const auto dx = std::abs(static_cast<std::int64_t>(b.x) - a.x);
		const auto dy = std::abs(static_cast<std::int64_t>(b.y) - a.y);

		auto decision = dy - dx;
		auto position = a;

		while (position != b)
		{
			if (decision == 0)
			{
				// Crossing a corner: check both neighboring cells.
				if (blocked({position.y, position.x + step_x})
						|| blocked({position.y + step_y, position.x}))
					return false;

				position.x += step_x;
				position.y += step_y;
				decision += 2 * (dy - dx);
			}
			else if (decision < 0)
			{
				position.x += step_x;
				decision += 2 * dy;
			}
			else
			{
				position.y += step_y;
				decision -= 2 * dx;
			}

			// Neither endpoint is checked for opacity.
			if (position != b && blocked(position))
				return false;
		}

		return true;
	}

	void update_vision(
			entt::registry& registry,
			const World::Grid& grid,
			entt::entity entity)
	{
		const auto& position =
			registry.get<Component::Value::Position>(entity).value;

		constexpr double vision_range = 10.0;
		const auto nearby_positions =
			World::nearby(position, vision_range);

		if (!registry.all_of<Component::Runtime::Vision>(entity))
			registry.emplace<Component::Runtime::Vision>(entity);

		auto& vision_cache =
			registry.get<Component::Runtime::Vision>(entity);

		auto& visible_positions = vision_cache.visible_positions;
		auto& visible_entities = vision_cache.visible_entities;

		visible_positions.clear();
		visible_entities.clear();

		for (const auto& np : nearby_positions)
		{
			if (!los(registry, grid, position, np))
				continue;

			visible_positions.push_back(np);

			const auto entities = Entity::find_all(
					registry,
					Component::Value::Position{np});

			for (const auto e : entities)
			{
				// TODO: Apply invisibility and other perception rules.
				visible_entities.push_back(e);
			}
		}

		vision_cache.seen_positions.insert(
				visible_positions.begin(),
				visible_positions.end());
	}
}
