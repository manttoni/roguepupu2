#include <cassert>
#include <string>
#include "external/entt.hpp"
#include "domain/Actor.hpp"
#include "domain/Effect.hpp"
#include "domain/Target.hpp"
#include "database/EntityFactory.hpp"
#include "systems/position/MovementSystem.hpp"
#include "utils/ECS.hpp"

namespace EffectSystem
{
	// Actor causes some Effect to Target
	void resolve_effect(entt::registry& registry, const Actor& actor, const Effect& effect, const Target& target)
	{
		assert(target.type != Target::Type::None);
		switch (effect.type)
		{
			case Effect::Type::CreateEntity:
				{
					const auto entity = EntityFactory::instance().create_entity(registry, effect.entity_id);
					switch (target.type)
					{
						case Target::Type::Cell:
							MovementSystem::move(registry, entity, target.position);
							break;
						case Target::Type::Self: // add to inventory
							.
						default:
							break;
					}
				}
				break;
			case Effect::Type::DestroyEntity:
				switch (target.type)
				{
					case Target::Type::Self:
						ECS::destroy_entity(registry, target.entity);
						ECS::add_event(registry, {
								.type = Event::Type::DestroyEntity,
								.actor = actor.entity,
								.target = target.entity
								});
						break;
					default:
						break;
				}
				break;
			case Effect::Type::Transition:
				if (target.position != Position{})
					registry.emplace_or_replace<Position>(target.entity, target.position);
				break;
			case Effect::Type::SetFGColor:
				registry.emplace_or_replace<FGColor>(target.entity, effect.fgcolor);
				LightingSystem::reset_lights(registry, ECS::get_active_cave(registry).get_idx());
				break;
			default:
				break;
		}
	}
};
