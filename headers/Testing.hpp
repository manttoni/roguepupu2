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
	}
};

/* TESTING PATHFINDING ALGORITHM */
void test_pathfinding()
{
	for (const auto& [map, expected_path_size] : test_maps)
	{
		Cave test_cave(map, 25);
		const auto& cells = test_cave.get_cells();
		const auto& path = test_cave.find_path(cells[0], cells[test_cave.get_size() - 1]);

		assert(test_cave.get_size() == map.size());
		assert(cells.size() == map.size());
		assert(path.size() == expected_path_size);
	}
}

void test_utils()
{
	assert(Random::randreal(0.1, 0.2) <= 0.2);
	assert(Math::map(1, 0, 1, 1, 10) == 10);
	assert(Math::map(0,0,1,1,10) == 1);
}

void test()
{
	test_pathfinding();
	test_utils();
}
