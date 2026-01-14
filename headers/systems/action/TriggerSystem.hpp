#pragma once

#include "domain/Trigger.hpp"
#include "external/entt/fwd.hpp"

namespace TriggerSystem
{
	void resolve_trigger(entt::registry& registry, Trigger trigger, const entt::entity owner, const entt::entity triggerer);
};
