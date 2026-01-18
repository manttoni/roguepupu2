#pragma once

#include "domain/Trigger.hpp"
#include "external/entt/fwd.hpp"

namespace TriggerSystem
{
	void resolve_trigger(entt::registry& registry, const Trigger& trigger, const Event& event);
};
