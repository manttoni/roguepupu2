#pragma once

#include <vector>
#include <limits>
#include <string>

struct Location
{
	struct Entity
	{
		std::string id = "";
		size_t amount = 1;
		double radial_distance = -1;
	};
	std::string id = "";
	double chance = 0;
	double radius = 0;
	double radial_distance = -1;
	std::vector<Entity> entities;
	bool unique = false;
};

class LocationDatabase
{
	private:
		std::vector<Location> locations;
	public:
		LocationDatabase();
		std::vector<Location> get_random_set() const;
		void read_locations();
};
