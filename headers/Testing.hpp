#pragma once

#include <map>
#include <string>
#include <cassert>

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
	Log::log("Testing pathfinding");
	for (const auto& [map, path_size] : test_maps)
	{
		Area test("test", map, 25);
		const auto& cells = test.get_cells();
		const auto& path = test.find_path(cells[0], cells[test.get_size() - 1]);

		assert(test.get_size() == map.size());
		assert(cells.size() == map.size());
		assert(path.size() == path_size);
	}
	Log::log("Pathfinding tests succesful");
}

void test()
{
	test_pathfinding();
	Log::log("All tests succesful");
}
