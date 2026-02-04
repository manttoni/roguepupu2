#include <stddef.h>
#include <stdint.h>
#include <vector>
#include <string>
#include <random>
#include <utility>

#include "systems/state/InventorySystem.hpp"
#include "systems/items/LootSystem.hpp"
#include "external/entt/entt.hpp"
#include "utils/Random.hpp"
#include "domain/LootTable.hpp"
#include "database/LootTableDatabase.hpp"
#include "database/EntityFactory.hpp"
#include "external/entt/entity/fwd.hpp"

namespace LootSystem
{
	std::vector<entt::entity> get_loot(entt::registry& registry, const std::string& table_id)
	{
		const LootTable& table = registry.ctx().get<LootTableDatabase>().get_loot_table(table_id);
		if (table.chance < Random::randreal(0, 1) || table.item_ids.empty())
			return {};
		std::vector<entt::entity> loot;
		const size_t amount = Random::randsize_t(table.amount.first, table.amount.second);

		if (table.amount.first < SIZE_MAX && table.item_ids.size() == table.weights.size())
		{	// amount is defined and weights are done correctly
			std::discrete_distribution<size_t> dist(table.weights.begin(), table.weights.end());
			for (size_t i = 0; i < amount; ++i)
			{
				const auto& id = table.item_ids[dist(Random::rng())];
				const auto item = EntityFactory::instance().create_entity(registry, id);
				loot.push_back(item);
			}
		}
		else if (table.amount.first < SIZE_MAX)
		{	// Implicit way of defining "give x amount from this list with equal chances"
			for (size_t i = 0; i < amount; ++i)
			{
				const size_t rand_idx = Random::randsize_t(0, table.item_ids.size() - 1);
				const auto& id = table.item_ids[rand_idx];
				const auto item = EntityFactory::instance().create_entity(registry, id);
				loot.push_back(item);
			}
		}
		else
		{	// Implicit way of defining "give one of each"
			for (const auto& id : table.item_ids)
			{
				const auto item = EntityFactory::instance().create_entity(registry, id);
				loot.push_back(item);
			}
		}
		return loot;
	}
	std::vector<entt::entity> get_loot(entt::registry& registry, const std::vector<std::string>& table_ids)
	{
		std::vector<entt::entity> loot;
		for (const auto& id : table_ids)
		{
			const auto l = get_loot(registry, id);
			loot.insert(loot.end(), l.begin(), l.end());
		}
		return loot;
	}
	void give_loot(entt::registry& registry, const entt::entity entity, const std::string& table_id)
	{
		for (const auto item : get_loot(registry, table_id))
			InventorySystem::add_item(registry, entity, item);
	}
	void give_loot(entt::registry& registry, const entt::entity entity, const std::vector<std::string>& table_ids)
	{
		for (const auto& id : table_ids)
			give_loot(registry, entity, id);
	}
};
