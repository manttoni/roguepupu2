#include "systems/Attitude.hpp"
#include "components/Component.hpp"
#include "external/entt/entt.hpp"


                namespace System::Attitude
            
{
	Attitude attitude(
			const entt::registry& registry,
			entt::entity observer,
			entt::entity target)
	{
		const auto& o_faction = registry.get<Component::Value::Faction>(observer).value;
		const auto& t_faction = registry.get<Component::Value::Faction>(target).value;

		if (o_faction == t_faction)
			return Attitude::Friendly;
		else
			return Attitude::Hostile;
	}

	bool is_enemy(
			const entt::registry& registry,
			entt::entity observer,
			entt::entity target)
	{
		const auto a = attitude(registry, observer, target);
		const auto b = attitude(registry, target, observer);

		return a == Attitude::Hostile || b == Attitude::Hostile;
	}

}
