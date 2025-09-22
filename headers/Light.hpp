#pragma once

#include <vector>
#include <cstdint>
#include "Color.hpp"

class Cave;

class Light
{
	private:
		Color color;
		double radius;

	public:
		Color get_color() const { return color; }
		double get_radius() const { return radius; }
		void set_color(const Color& color) { this->color = color; }
		void set_radius(const double radius) { this->radius = radius; }

		Light();
		Light(const Color& color, const double radius);

		void shine(Cave& cave, const size_t idx);

};
