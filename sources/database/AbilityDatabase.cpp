#include <stddef.h>
#include <nlohmann/detail/iterators/iter_impl.hpp>
#include <nlohmann/detail/iterators/iteration_proxy.hpp>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <string>
#include <filesystem>
#include <map>

#include "utils/Parser.hpp"
#include "database/AbilityDatabase.hpp"
#include "domain/Ability.hpp"
#include "domain/Effect.hpp"

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
	nlohmann::json definitions = Parser::read_file(path);
	const std::string category = path.stem().filename();
	add_abilities(definitions, category);
}

void AbilityDatabase::add_abilities(nlohmann::json& definitions, const std::string& category)
{
	for (const auto& [id, data] : definitions.items())
	{
		Ability ability;
		ability.id = id;
		ability.category = category;
		if (data.contains("cooldown"))
			ability.cooldown = data["cooldown"].get<size_t>();
		ability.effect = Parser::parse_effect(data["effect"]);
		database[id] = ability;
	}
}
