#include <chrono>
#include <thread>
#include "systems/VisualEffectSystem.hpp"
#include "Components.hpp"
#include "ECS.hpp"
#include "Cell.hpp"

namespace VisualEffectSystem
{
	void flash_entity(entt::registry& registry, const entt::entity entity, const Color& fgcolor, const size_t ms)
	{
		if (!registry.all_of<FGColor>(entity))
			return;
		Color original = ECS::get_color(registry, entity);
		registry.get<FGColor>(entity).color = fgcolor;
		Cell* cell = ECS::get_cell(registry, entity);
		registry.ctx().get<Renderer>().render_cell(*cell);
		std::this_thread::sleep_for(std::chrono::milliseconds(ms));
		registry.get<FGColor>(entity).color = original;
		registry.ctx().get<Renderer>().render_cell(*cell);
	}
	void damage_flash(entt::registry& registry, const entt::entity entity)
	{
		flash_entity(registry, entity, Color(500, 0, 0));
	}
};
