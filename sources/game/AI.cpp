#include "game/AI.hpp"
#include "external/entt/entt.hpp"
#include "game/component/Component.hpp"

namespace Game::AI
{
	Event::Any get_event(const Simulation& simulation, const entt::entity entity)
	{
		(void) simulation;
		(void) entity;
		return Event::Null{};
	}
}
