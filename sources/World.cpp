#include "World.hpp"
#include "Cave.hpp"

size_t World::add_cave(Cave& cave)
{
	cave.set_idx(caves.size());
	caves.push_back(cave);
	return cave.get_idx();
}

Cave& World::get_cave(const size_t idx)
{
	if (idx >= caves.size())
		Log::error("Cave does not exist");
	return caves[idx];
}

const Cave& World::get_cave(const size_t idx) const
{
	if (idx >= caves.size())
		Log::error("Cave does not exist");
	return caves[idx];
}
