#include <nlohmann/json.hpp>
#include <stddef.h>
#include <nlohmann/detail/iterators/iter_impl.hpp>
#include <nlohmann/detail/iterators/iteration_proxy.hpp>
#include <nlohmann/json_fwd.hpp>
#include <filesystem>
#include <vector>
#include <string>
#include <algorithm>
#include <map>
#include <utility>

#include "database/EntityFactory.hpp"
#include "utils/Parser.hpp"
#include "domain/LootTable.hpp"
#include "database/LootTableDatabase.hpp"

LootTableDatabase::LootTableDatabase()
{
	read_definitions();
}

void LootTableDatabase::read_definitions()
{
	const std::filesystem::path path = "data/loot_tables.json";
	nlohmann::json definitions = Parser::read_file(path);

	for (const auto& [id, data] : definitions.items())
	{
		LootTable lt;
		lt.id = id;

		// Parse ids and weights. If weights.size() and item_ids.size() dont match,
		// give one of each (max amount)
		if (data.contains("amount"))
			lt.amount = Parser::parse_range<size_t>(data["amount"]);
		if (data.contains("chance"))
			lt.chance = data["chance"].get<double>();
		if (data.contains("item_ids"))
			lt.item_ids = data["item_ids"].get<std::vector<std::string>>();
		if (data.contains("weights"))
			lt.weights = data["weights"].get<std::vector<double>>();
		if (data.contains("filter"))
			lt.item_ids = EntityFactory::instance().filter_entity_ids(data["filter"]);
		if (data.contains("item_weights"))
		{
			for (const auto& [id, weight] : data["item_weights"].items())
			{
				lt.item_ids.push_back(id);
				lt.weights.push_back(weight.get<double>());
			}
		}
		database[id] = lt;
	}
}
