#pragma once

#include <deque>
#include "Cave.hpp"  // for Cave

class World
{
	private:
		std::deque<Cave> caves;

	public:
		World();
		size_t add_cave(Cave& cave);
		Cave& get_cave(const size_t idx);
		const Cave& get_cave(const size_t idx) const;
};

