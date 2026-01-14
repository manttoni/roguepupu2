#include <cassert>
#include "Cave.hpp"
#include "ECS.hpp"
#include "systems/LightingSystem.hpp"
#include "systems/MovementSystem.hpp"
#include "systems/EffectSystem.hpp"
#include "entt.hpp"
#include "EntityFactory.hpp"
#include "Event.hpp"
#include "Components.hpp"
#include "Utils.hpp"

namespace EffectSystem
{
	void resolve_effect(entt::registry& registry, const Effect& effect, const Target& target)
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
