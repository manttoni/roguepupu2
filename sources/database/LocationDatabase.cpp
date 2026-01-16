//this file is probably unused
#include <filesystem>
#include <string>
#include <fstream>
#include "database/LocationDatabase.hpp"
#include "nlohmann/json.hpp"

LocationDatabase::LocationDatabase()
{
	read_locations();
}

void LocationDatabase::read_locations()
{
	const std::filesystem::path root = "data/world/locations";
	for (const auto& entry : std::filesystem::recursive_directory_iterator(root))
	{
		if (!entry.is_regular_file())
			continue;
		if (entry.path().extension() != ".json")
			continue;
		read_definitions(entry.path());
	}
}

void LocationDatabase::read_definitions(const std::filesystem::path& path)
{
	Log::log("Parsing " + path.string());
	std::ifstream file(path);
	if (!file.is_open())
		Log::error("Opening file failed: " + path.string());
	nlohmann::json definitions;
	file >> definitions;
	file.close();

	for (const auto& [id, data] : definitions.items())
	{
		Location location;
		location.id = id;
		if (path.stem().filename() == "natural" ||
			(data.contains("natural") && data["natural"].get<bool>() == true))
			location.natural = true;
		if (data.contains("chance"))
			location.chance = data["chance"].get<double>();
		if (data.contains("radius"))
			location.radius = data["radius"].get<double>();
		if (data.contains("radial_distance"))
			location.radial_distance = data["radial_distance"].get<double>();
		if (data.contains("amount"))
			location.amount = Parser::parse_range(data["amount"]);
		if (data.contains("entities"))
		{
			for (const auto& entry : data["entities"])
			{
				Location::Entity entity;

				// entity id(s)
				if (entry.contains("id"))
					entity.name_pool.push_back(entry["id"].get<std::string>());
				else if (entry.contains("filter"))
					entity.name_pool = EntityFactory::instance()
						.random_pool(entry["filter"]);
				if (entity.name_pool.empty())
					Log::error("Invalid id or filter: " + entry.dump(4));

				// entity amount range
				if (entry.contains("amount"))
					entity.amount = Parser::parse_range(entry["amount"]);

				// entity spawn position relative to location center
				if (entry.contains("position"))
				{
					const auto& position = entry["position"];
					if (position.contains("radial"))
						entity.position.radial = position["radial"].get<double>();
					if (position.contains("horizontal"))
						entity.position.horizontal = position["horizontal"].get<size_t>();
					if (position.contains("vertical"))
						entity.position.vertical = position["vertical"].get<size_t>();
				}
				location.entities.push_back(entity);
			}
		}
		locations.push_back(location);
	}
}

std::vector<Location> LocationDatabase::get_locations(const bool get_naturals, const size_t optional) const
{
	(void) get_naturals;
	std::vector<Location> natural_locations;
	size_t optional_added = 0;
	for (const auto& location : locations)
	{
		// Natural locations are used in cave generation
		//if (location.natural != get_naturals)
		//	continue;
		const size_t amount = Random::randsize_t(location.amount.first, location.amount.second);
		for (size_t i = 0; i < amount; ++i)
		{
			if (location.chance < 1.0)
			{
				if (optional_added == optional || Random::randreal(0.0, 1.0) > location.chance)
					continue;
				optional_added++;
			}
			natural_locations.push_back(location);
		}
	}
	return natural_locations;
}
