#include <cassert>
#include "Cave.hpp"
#include "ECS.hpp"
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
							if (target.cell == nullptr)
								Log::error("Creating on nullptr");
							MovementSystem::move(registry, entity, target.cell);
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
						ECS::destroy_entity(registry, target.entity); // wipe from existence
						break;
					default:
						break;
				}
				break;
			case Effect::Type::Transition:
				if (target.cell != nullptr)
					registry.emplace_or_replace<Position>(target.entity, target.cell);
				break;
			case Effect::Type::SetFGColor:
				registry.emplace_or_replace<FGColor>(target.entity, effect.fgcolor);
				ECS::get_active_cave(registry)->reset_lights();
				break;
			default:
				break;
		}
	}
};
