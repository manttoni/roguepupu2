#include <string>
#include "Ability.hpp"
#include "Cell.hpp"
#include "entt.hpp"
#include "EntityFactory.hpp"
#include "Utils.hpp"
#include "GameState.hpp"

Ability::Ability(const std::string& id, const std::string& category, const std::string& target, const size_t cooldown, const std::string& summon)
	: id(id), category(category), target(target), cooldown(cooldown), summon(summon), use_turn(0)
{}

void Ability::use(entt::registry& registry, Cell* target)
{
	if (target == nullptr)
		Log::error("Invalid ability target: " + id);
	if (!summon.empty())
		EntityFactory::instance().create_entity(registry, summon, target);
	use_turn = registry.ctx().get<GameState>().turn_number;
}

bool Ability::on_cooldown(const entt::registry& registry) const
{
	if (use_turn == 0)
		return false;
	const size_t turn_number = registry.ctx().get<GameState>().turn_number;
	return cooldown + use_turn >= turn_number;
}
