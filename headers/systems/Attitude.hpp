#pragma once

#include "external/entt/fwd.hpp"


                namespace System::Attitude
            
{
	enum class Attitude
	{
		Friendly, Neutral, Hostile
	};

	Attitude attitude(
			const entt::registry& registry,
			entt::entity observer,
			entt::entity target);

	bool is_enemy(
			const entt::registry& registry,
			entt::entity observer,
			entt::entity target);
}
