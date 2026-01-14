#pragma once

#include "entt.hpp"
#include "Event.hpp"

namespace TriggerSystem
{
	void resolve_trigger(entt::registry& registry, Trigger trigger, const entt::entity owner, const entt::entity triggerer);
};
