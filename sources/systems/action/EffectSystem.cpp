#include <string>

#include "external/entt/entt.hpp"
#include "domain/Effect.hpp"
#include "domain/Target.hpp"
#include "database/EntityFactory.hpp"
#include "systems/position/MovementSystem.hpp"
#include "systems/state/InventorySystem.hpp"
#include "utils/ECS.hpp"
#include "utils/Error.hpp"
#include "domain/Event.hpp"
#include "domain/Position.hpp"
#include "external/entt/entity/fwd.hpp"

struct Actor;

namespace EffectSystem
{
	// Actor causes some Effect to Target
	void resolve_effect(entt::registry& registry, const Actor& actor, const Effect& effect, const Target& target)
	{
		switch (effect.type)
		{
			case Effect::Type::CreateEntity:
				{
					const auto entity = EntityFactory::instance().create_entity(registry, effect.entity_id);
					if (target.position.is_valid())
						// "Effect entity is created in target position"
						MovementSystem::move(registry, entity, target.position);
					else if (target.entity != entt::null)
						// "Effect entity is created and added to target entity inventory"
						InventorySystem::add_item(registry, target.entity, entity);
					else
						Error::fatal("Effect target is invalid");
				}
				break;
			default:
				Error::fatal("Unhandled effect type: " + std::to_string(static_cast<int>(effect.type)));
		}

		ECS::queue_event(registry, Event(
					actor,
					effect,
					target
					));
	}
};
