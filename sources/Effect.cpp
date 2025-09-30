#include "Effect.hpp"

Effect::Effect(const Type type, const double radius, const size_t duration) : type(type), radius(radius), duration(duration) {}
Effect::Effect() {}
Effect::~Effect() {}
Effect::Effect(const Effect& other) : type(other.type), radius(other.radius), duration(other.duration) {}
Effect& Effect::operator=(const Effect& other)
{
	if (this != &other)
	{
		type = other.type;
		radius = other.radius;
		duration = other.duration;
	}
	return *this;
}
