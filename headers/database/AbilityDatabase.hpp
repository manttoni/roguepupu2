#pragma once

#include <string>
#include <filesystem>
#include <map>
#include <fstream>
#include "nlohmann/json.hpp"

struct Ability;
class AbilityDatabase
{
	private:
		std::map<std::string, Ability> abilities;

	public:
		AbilityDatabase();
		Ability get_ability(const std::string& id) const { return abilities.at(id); }
		void read_abilities();
		void read_definitions(const std::filesystem::path& path);
		void add_abilities(nlohmann::json& definitions, const std::string& category);
};
