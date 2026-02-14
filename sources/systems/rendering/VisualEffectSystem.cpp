#include <bits/chrono.h>
#include <stddef.h>
#include <thread>

#include "UI/UI.hpp"
#include "systems/rendering/VisualEffectSystem.hpp"
#include "systems/rendering/RenderingSystem.hpp"
#include "components/Components.hpp"
#include "utils/ECS.hpp"
#include "domain/Color.hpp"
#include "domain/Position.hpp"
#include "external/entt/entity/fwd.hpp"
#include "external/entt/entity/registry.hpp"

namespace VisualEffectSystem
{
	void flash_entity(entt::registry& registry, const entt::entity entity, const Color& fgcolor, const size_t ms)
	{
		if (registry.ctx().get<GameState>().test_run == true)
			return;
		Color original = ECS::get_fgcolor(registry, entity);
		registry.get<FGColor>(entity).color = fgcolor;
		const Position& position = registry.get<Position>(entity);
		RenderingSystem::render_cell(registry, position);
		UI::instance().update();
		std::this_thread::sleep_for(std::chrono::milliseconds(ms));
		registry.get<FGColor>(entity).color = original;
		RenderingSystem::render_cell(registry, position);
	}
	void damage_flash(entt::registry& registry, const entt::entity entity)
	{
		flash_entity(registry, entity, Color(500, 0, 0));
	}
};
