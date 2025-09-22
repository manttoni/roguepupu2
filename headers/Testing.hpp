#pragma once

#include <map>
#include <string>
#include <cassert>
#include "Utils.hpp"

/* TEST MAPS */
// key = map as string, value = length of path from top left to bot right
const std::map<std::string, size_t> test_maps =
{
	{
		"ffffffffffffffwwfwfwwwfff"
		"wffffwffwfwfwwwfwwwffwfff"
		"fffffwfwwfffffffffwffwffw"
		"fwfffffwwffffffwffwwffffw"
		"ffwfffffwfwwwffffwwffwfww"
		"ffffwffwffffffwfwffffffwf"
		"fffffffffffffffffwffwffwf"
		"ffffwwfffffffwfwfwfwwwwwf"
		"wffffffwfffwfffffwfwwffwf"
		"fwffwfwwfffffffwwffwfffff"
		"fwfwfwfffwfwwfwffffffffff"
		"ffffwwfffffffffffwfffffff"
		"wfffffffffffffffffffffwff"
		"ffffwffwfwfffwffwffffwffw"
		"wwffwfffffffwffffwfffwfww"
		"fwfffffffwfwwfwffffffwfwf"
		"fwfwffwffffwffffffwffwfwf"
		"wffwfwfffwffffwfwffwffffw"
		"wffffffffwwwffwfffffwwfff"
		"wwwffffwfffwffffffwffwwff"
		"ffffwwfwfffffwffffwfffwff"
		"ffffffffwwwfwfffffffffwff"
		"fffffwwfffwfffwfwwwffffff"
		"fwffwfffwffffffffffffffff"
		"fffwwffwffwffffffwfwfwfff",
		32
	},
	{
		"wfffffffffffffwwfwfwwwfff"
		"wffffwffwfwfwwwfwwwffwfff"
		"fffffwfwwfffffffffwffwffw"
		"fwfffffwwffffffwffwwffffw"
		"ffwfffffwfwwwffffwwffwfww"
		"ffffwffwffffffwfwffffffwf"
		"fffffffffffffffffwffwffwf"
		"ffffwwfffffffwfwfwfwwwwwf"
		"wffffffwfffwfffffwfwwffwf"
		"fwffwfwwfffffffwwffwfffff"
		"fwfwfwfffwfwwfwffffffffff"
		"ffffwwfffffffffffwfffffff"
		"wfffffffffffffffffffffwff"
		"ffffwffwfwfffwffwffffwffw"
		"wwffwfffffffwffffwfffwfww"
		"fwfffffffwfwwfwffffffwfwf"
		"fwfwffwffffwffffffwffwfwf"
		"wffwfwfffwffffwfwffwffffw"
		"wffffffffwwwffwfffffwwfff"
		"wwwffffwfffwffffffwffwwff"
		"ffffwwfwfffffwffffwfffwff"
		"ffffffffwwwfwfffffffffwff"
		"fffffwwfffwfffwfwwwffffff"
		"fwffwfffwffffffffffffffff"
		"fffwwffwffwffffffwfwfwfff",
		0
	},
	{
		"ffffffffffffffwwfwfwwwfff"
		"wffffwffwfwfwwwfwwwffwfff"
		"fffffwfwwfffffffffwffwffw"
		"fwfffffwwffffffwffwwffffw"
		"ffwfffffwfwwwffffwwffwfww"
		"ffffwffwffffffwfwffffffwf"
		"fffffffffffffffffwffwffwf"
		"ffffwwfffffffwfwfwfwwwwwf"
		"wffffffwfffwfffffwfwwffwf"
		"fwffwfwwfffffffwwffwfffff"
		"fwfwfwfffwfwwfwffffffffff"
		"ffffwwfffffffffffwfffffff"
		"wfffffffffffffffffffffwff"
		"ffffwffwfwfffwffwffffwffw"
		"wwffwfffffffwffffwfffwfww"
		"fwfffffffwfwwfwffffffwfwf"
		"fwfwffwffffwffffffwffwfwf"
		"wffwfwfffwffffwfwffwffffw"
		"wffffffffwwwffwfffffwwfff"
		"wwwffffwfffwffffffwffwwff"
		"ffffwwfwfffffwffffwfffwff"
		"ffffffffwwwfwfffffffffwff"
		"fffffwwfffwfffwfwwwffffff"
		"fwffwfffwffffffffffffffff"
		"fffwwffwffwffffffwfwfwffw",
		0
	}
};

void test_cave()
{
	for (const auto& [map, expected_path_size] : test_maps)
	{
		// pathfinding
		Cave test_cave(map, 25);
		const auto& test_cave_cells = test_cave.get_cells();
		const auto& path = test_cave.find_path(0, test_cave.get_size() - 1);

		assert(test_cave.get_size() == map.size());
		assert(test_cave_cells.size() == map.size());
		assert(path.size() == expected_path_size);
		try
		{
			const auto& path = test_cave.find_path(-1, test_cave.get_size());
		}
		catch (std::exception& e)
		{
		}
	}

	{
		size_t test_size = 25;
		Cave test_cave(test_maps.begin()->first, test_size);

		// neighbors
		for (size_t i = 0; i < test_cave.get_size(); ++i)
		{
			size_t expected = 8;
			const auto& neighbor_ids = test_cave.get_nearby_ids(i, 1.5);
			size_t iy = i / test_size;
			size_t ix = i % test_size;
			if (iy == 0 || iy == test_size - 1)
				expected -= 3;
			if (ix == 0 || ix == test_size - 1)
				expected -= expected == 5 ? 2 : 3;
			assert(neighbor_ids.size() == expected);
		}
	}

	{
		// has_access test, aka collision detection
		std::string map =
			"wwwwwww"
			"wfwfffw"
			"wffwffw" // <- testing with third index on this row (16)
			"wfffffw"
			"wfffffw"
			"wfffffw"
			"wwwwwww";
		size_t test_size = 7;
		Cave test_cave(map, test_size);
		size_t test_idx = 16; // "main character"

		const auto& neighbor_ids = test_cave.get_nearby_ids(test_idx, 1.5);
		const std::vector<size_t> inaccessible = {8, 9, 10, 17, 24};
		for (const size_t nidx : neighbor_ids)
		{
			bool is_accessible = !Utils::contains(inaccessible, nidx);
			bool has_access_return = test_cave.has_access(test_idx, nidx);
			assert(is_accessible == has_access_return);
		}
	}
}

void test_utils()
{
	assert(Random::randreal(0.1, 0.2) <= 0.2);
	assert(Math::map(1, 0, 1, 1, 10) == 10);
	assert(Math::map(0,0,1,1,10) == 1);
}

void test_get_nearby_ids()
{
	std::string map =
		"ff"
		"ff";
	Cave cave(map, 2);
	assert(cave.get_nearby_ids(0, 2).size() == 3);
}

void test()
{
	test_utils();
	test_cave();
	test_get_nearby_ids();
}
