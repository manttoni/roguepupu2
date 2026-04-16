#include <bits/chrono.h>
#include <stddef.h>
#include <thread>

#include "UI/UI.hpp"
#include "systems/perception/VisionSystem.hpp"
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
		// Assuming you never want to flash any entity if it is out of sight of player
		if (VisionSystem::has_vision(registry, ECS::get_player(registry), entity) == false)
			return;

		if (registry.ctx().get<GameState>().test_run == true)
			return;
		Color original = ECS::get_fgcolor(registry, entity);
		registry.replace<Color>(entity, fgcolor);
		const Position& position = registry.get<Position>(entity);
		RenderingSystem::render_cell(registry, position);
		UI::instance().update();
		std::this_thread::sleep_for(std::chrono::milliseconds(ms)); // todo: some kind of time util
		registry.replace<Color>(entity, original);
		RenderingSystem::render_cell(registry, position);
	}
	void damage_flash(entt::registry& registry, const entt::entity entity)
	{
		flash_entity(registry, entity, Color(500, 0, 0));
	}
};
