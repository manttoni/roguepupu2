#pragma once

#include <map>
#include <string>
#include "domain/LootTable.hpp"
#include "utils/Log.hpp"

class LootTableDatabase
{
	private:
		std::map<std::string, LootTable> database;

	public:
		LootTableDatabase();
		void read_definitions();
		const LootTable& get_loot_table(const std::string& table_id) const
		{
			if (!database.contains(table_id))
				Log::error("LootTableDatabase does not contain: " + table_id);
			return database.at(table_id);
		}
};
