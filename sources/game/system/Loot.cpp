#include <iterator>

#include "game/entity/Entity.hpp"
#include "game/system/Loot.hpp"
#include "game/component/Value.hpp"
#include "game/component/Resource.hpp"
#include "utils/Random.hpp"

namespace Game::System::Loot
{
	Event::List generate(
		entt::registry& registry,
		const EntityDatabase& database,
		const entt::entity recipient,
		const entt::entity loot_table)
	{
		auto& gold_budget =
			registry.get<Component::Resource::GoldBudget>(loot_table);
		auto& weight_budget =
			registry.get<Component::Resource::WeightBudget>(loot_table);
		auto& count_budget =
			registry.get<Component::Resource::CountBudget>(loot_table);
		const auto& item_ids =
			registry.get<Component::List::EntityIDs>(loot_table);

		std::vector<Entry> entries;
		entries.reserve(item_ids.values.size());

		const auto* position =
			registry.try_get<Component::Value::Position>(recipient);
		for (const auto& id : item_ids.values)
		{
			entries.push_back({
				id,
				Random::at(
					Random::hash_seed(id),
					position ? position->value : World::GlobalPosition{})});
		}

		std::sort(entries.begin(), entries.end(),
			[](const auto& a, const auto& b)
			{
			return a.priority > b.priority;
			});

		Event::List events;

		const bool allow_duplicates =
			registry.get<Component::Value::AllowDuplicates>(loot_table).value;

		for (const auto& entry : entries)
		{
			if (count_budget.current == 0)
				break;

			const auto prototype =
				Entity::create(registry, database, entry.id);
			if (!registry.valid(prototype))
				continue;

			const auto prototype_gold =
				registry.get<Component::Value::GoldValue>(prototype).value;
			const auto prototype_weight =
				registry.get<Component::Value::Weight>(prototype).value;

			registry.destroy(prototype);

			while (
				count_budget.current > 0 &&
				gold_budget.current >= prototype_gold &&
				weight_budget.current >= prototype_weight)
			{
				const auto item =
					Entity::create(registry, database, entry.id);
				if (!registry.valid(item))
					break;

				gold_budget.current -= prototype_gold;
				weight_budget.current -= prototype_weight;
				--count_budget.current;

				events.push_back(
					Event::ReceiveItem{
					.entity = recipient,
					.item = item
					});

				if (!allow_duplicates)
					break;
			}
		}

		return events;
	}

	Event::List generate(
		entt::registry& registry,
		const EntityDatabase& database,
		const entt::entity recipient,
		const Component::List::Base<Entity::Definition::ID>* loot_tables)
	{
		if (!loot_tables || !registry.valid(recipient))
			return {};

		Event::List events;

		for (const auto& id : loot_tables->values)
		{
			const auto loot_table = Entity::create(registry, database, id);
			if (!registry.valid(loot_table))
				continue;

			auto loot_table_events = generate(
				registry,
				database,
				recipient,
				loot_table);
			events.insert(
				events.end(),
				std::make_move_iterator(loot_table_events.begin()),
				std::make_move_iterator(loot_table_events.end()));

			registry.destroy(loot_table);
		}

		return events;
	}
}

