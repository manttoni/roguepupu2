#pragma once

class Cave;
namespace LiquidSystem
{
	void simulate_liquids(Cave& cave);
	double get_liquids_volume(const Cave& cave);
};
