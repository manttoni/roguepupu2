#pragma once

class Cave;
namespace EnvironmentSystem
{
	void simulate_environment(Cave* cave);
	void simulate_liquids(Cave* cave);
	double get_liquids_volume(Cave * cave);
};
