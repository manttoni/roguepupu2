#include <cassert>
#include <string>
#include "external/entt/entt.hpp"
#include "domain/Actor.hpp"
#include "domain/Effect.hpp"
#include "domain/Target.hpp"
#include "database/EntityFactory.hpp"
#include "systems/position/MovementSystem.hpp"
#include "systems/state/InventorySystem.hpp"
#include "utils/ECS.hpp"

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
						Log::error("Effect target is invalid");
				}
				break;
			case Effect::Type::Transition:
				if (actor.entity != entt::null && target.position.is_valid())
					// Dont use move() here to not trigger infinite loop
					// "Actor entity transitions to target position"
					// Transitioning is like moving but can be trans-cave
					registry.emplace_or_replace<Position>(actor.entity, target.position);
				break;
			case Effect::Type::DestroyEntity:
				// "Target entity is destroyed"
				registry.destroy(target.entity);
				break;
			default:
				Log::error("Unhandled effect type: " + std::to_string(static_cast<int>(effect.type)));
		}

		ECS::queue_event(registry, Event(
					actor,
					effect,
					target
					));
	}
};
