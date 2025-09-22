#include "Light.hpp"
#include "Cave.hpp"
#include "Color.hpp"

Light::Light() : color(Color()), radius(0.0) {}
Light::Light(const Color& color, const double radius) : color(color), radius(radius) {}
void Light::shine(Cave& cave, const size_t idx)
{
	auto affected = cave.get_nearby_ids(idx, radius);
	affected.push_back(idx);
	auto& cells = cave.get_cells();

	for (size_t i = 0; i < affected.size(); ++i)
	{
		if (!cave.has_vision(idx, affected[i]))
			continue;
		cells[affected[i]].add_light(color);
	}
}
