#include <string>
#include <filesystem>
#include "utils/Parser.hpp"
#include "utils/Log.hpp"
#include "database/AbilityDatabase.hpp"

AbilityDatabase::AbilityDatabase()
{
	read_abilities();
}

void AbilityDatabase::read_abilities()
{
	const std::filesystem::path root = "data/abilities";
	for (const auto& entry : std::filesystem::recursive_directory_iterator(root))
	{
		if (!entry.is_regular_file())
			continue;
		if (entry.path().extension() != ".json")
			continue;
		read_definitions(entry.path());
	}
}

void AbilityDatabase::read_definitions(const std::filesystem::path& path)
{
	Log::log("Parsing " + path.string());
	std::ifstream file(path);
	if (!file.is_open())
		Log::error(std::string("Opening file failed: ") + path.string());
	if (file.peek() == std::ifstream::traits_type::eof())
	{
		Log::log("Empty file: " + path.string());
		file.close();
		return;
	}
	nlohmann::json definitions;
	file >> definitions;
	file.close();
	const std::string category = path.stem().filename(); // innate or spells
	add_abilities(definitions, category);
}

void AbilityDatabase::add_abilities(nlohmann::json& definitions, const std::string& category)
{
	for (const auto& [id, data] : definitions.items())
	{
		Ability ability;
		ability.id = id;
		if (category == "innate" || (data.contains("innate") && data["innate"].get<bool>() == true))
			ability.innate = true;
		if (data.contains("cooldown"))
			ability.cooldown = data["cooldown"].get<size_t>();
		ability.effect = Parser::parse_effect(data["effect"]);
		abilities[id] = ability;
	}
}
