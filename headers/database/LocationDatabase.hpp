#pragma once

#include <utility>
#include <vector>
#include <limits>
#include <string>
#include <filesystem>
#include "Utils.hpp"
#include "Parser.hpp"

struct Location
{
	struct Entity
	{
		struct Position
		{
			double radial = 0; // f.e. for forming a circle
			size_t vertical = 0; // this and next can form a rectangle
			size_t horizontal = 0; // f.e. a house/room
			Vec2 direction; // maybe needed f.e. setting a door in right place
		};
		std::vector<std::string> name_pool; // names of entities that can spawn
		std::pair<size_t, size_t> amount{1, 1};
		Position position{}; // info about spawn position
	};
	std::string id = "";
	std::pair<size_t, size_t> amount{1, 1};
	double chance = 1.0;
	double radius = 5; // other locations cannot spawn within
	double radial_distance = -1; // default = spawn anywhere
	std::vector<Entity> entities; // list of spawn entries
	bool natural = false; // natural locations affect cave generation
};

class LocationDatabase
{
	private:
		std::vector<Location> locations;
	public:
		LocationDatabase();
		void read_locations();
		void read_definitions(const std::filesystem::path& path);
		std::vector<Location> get_locations(const bool get_naturals, const size_t optional = SIZE_MAX) const;
};
