#include <cassert>
#include "domain/Cell.hpp"        // for Cell
#include "utils/Log.hpp"
#include "domain/LiquidMixture.hpp"

Cell::Cell(const size_t idx, const Cell::Type type) :
	idx(idx),
	glyph(L' ')
{
	switch (type)
	{
		case Cell::Type::Rock:
			density = CELL_DENSITY_MAX;
			break;
		case Cell::Type::Floor:
			density = 0;
			break;
		default:
			Log::error("Don't use other types here");
	}
}

void Cell::reduce_density(const double amount)
{
	density -= amount;
}

double Cell::get_liquid_level() const
{
	assert(density <= 0);
	return density + liquid_mixture.get_volume();
}

