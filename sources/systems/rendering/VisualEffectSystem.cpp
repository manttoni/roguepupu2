#include <chrono>
#include <thread>
#include "systems/rendering/VisualEffectSystem.hpp"
#include "systems/rendering/RenderingSystem.hpp"
#include "components/Components.hpp"
#include "utils/ECS.hpp"

namespace VisualEffectSystem
{
	void flash_entity(entt::registry& registry, const entt::entity entity, const Color& fgcolor, const size_t ms)
	{
		Color original = ECS::get_color(registry, entity);
		registry.get<FGColor>(entity).color = fgcolor;
		const Position& position = registry.get<Position>(entity);
		RenderingSystem::render_cell(registry, position, true);
		std::this_thread::sleep_for(std::chrono::milliseconds(ms));
		registry.get<FGColor>(entity).color = original;
		RenderingSystem::render_cell(registry, position, true);
	}
	void damage_flash(entt::registry& registry, const entt::entity entity)
	{
		flash_entity(registry, entity, Color(500, 0, 0));
	}
};
