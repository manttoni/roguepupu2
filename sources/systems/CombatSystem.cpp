#include <vector>
#include <chrono>
#include <thread>
#include "Components.hpp"
#include "entt.hpp"
#include "systems/ActionSystem.hpp"
#include "systems/CombatSystem.hpp"
#include "systems/DamageSystem.hpp"
#include "systems/EquipmentSystem.hpp"
#include "ECS.hpp"
#include "GameLogger.hpp"
#include "Utils.hpp"

namespace CombatSystem
{
	void attack(entt::registry& registry, const entt::entity attacker, const entt::entity defender)
	{
		(void) registry; (void) attacker; (void) defender;
	}
};
