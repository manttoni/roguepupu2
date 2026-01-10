#include <filesystem>
#include <string>
#include <fstream>
#include "LocationDatabase.hpp"
#include "Utils.hpp"
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
		if (data.contains("unique"))
			location.unique = data["unique"].get<bool>();
		if (data.contains("entities"))
		{
			for (const auto& entry : data["entities"])
			{
				Location::Entity entity = {.id = entry["id"].get<std::string>()};
				if (entry.contains("amount"))
					entity.amount = entry["amount"].get<size_t>();
				if (entry.contains("radial_distance"))
					entity.radial_distance = entry["radial_distance"].get<double>();
				location.entities.push_back(entity);
			}
		}
		locations.push_back(location);
	}

}

std::vector<Location> LocationDatabase::get_random_set() const
{
	std::vector<Location> set;
	for (const auto& location : locations)
	{
		while (location.chance >= Random::randreal(0, 1))
		{
			set.push_back(location);
			if (location.unique || set.size() > 10)
				break;
		}
	}
	return set;
}
