#include <stddef.h>
#include <format>
#include <string>

#include "domain/Cell.hpp"        // for Cell
#include "utils/Error.hpp"
#include "ncurses/Color.hpp"
#include "utils/Random.hpp"

namespace Domain
{
Cell::Cell(const Cell::Type type) : type(type)
{
}

void Cell::reduce_density(const double amount)
{
	density -= amount;
	if (density <= 0)
		type = Type::Floor;
}

std::string Cell::to_string() const
{
	std::string string = "";
	switch (get_type())
	{
		case Type::Rock:
			string += "Rock | Density: " + std::format("{:.2f}", density);
			break;
		case Type::Floor:
			string += "Floor | Elevation: " + std::format("{:.2f}", density);
			break;
		default:
			break;
	}
	return string;
}
} // namespace Domain
