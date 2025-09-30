#include "Light.hpp"
#include "Cave.hpp"
#include "Color.hpp"

Light::Light()
	: Effect(Effect::Type::LIGHT, 0, 0), color(Color())
{}
Light::Light(const double radius, const size_t duration, const Color& color)
	: Effect(Effect::Type::LIGHT, radius, duration), color(color)
{}
Light::Light(const Light& other)
	: Effect(Effect::Type::LIGHT, other.radius, other.duration), color(other.color)
{}
Light& Light::operator=(const Light& other)
{
	if (this != &other)
	{
		radius = other.radius;
		duration = other.duration;
		color = other.color;
	}
	return *this;
}
void Light::trigger(Cave& cave, const size_t idx)
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
