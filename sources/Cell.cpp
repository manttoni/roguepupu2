#include <cassert>
#include "Cell.hpp"        // for Cell
#include "Color.hpp"       // for Color
#include "ColorPair.hpp"   // for ColorPair
#include "Unicode.hpp"

Cell::Cell(const size_t idx, const Cell::Type type) :
	idx(idx),
	glyph(L' '),
	seen(false)
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
	assert(density > 0);
	return density + liquid_mixture.get_volume();
}

wchar_t Cell::get_glyph() const
{
	if (glyph - L'0' > 0 && glyph - L'0' < 10)
		return glyph; // DevTools thing to show cell values
	if (liquid_mixture.get_volume() <= 0.01)
		return glyph;
	return liquid_mixture.get_glyph();
}

