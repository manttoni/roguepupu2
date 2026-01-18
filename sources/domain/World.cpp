#include <utility>
#include "domain/World.hpp"
#include "domain/Cave.hpp"

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

size_t World::new_cave(const size_t size, const Cell::Type fill)
{
	Cave cave(size, fill);
	const size_t idx = caves.size();
	cave.set_idx(idx);
	caves.push_back(std::move(cave));
	return idx;
}
