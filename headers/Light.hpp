#pragma once

#include <vector>
#include <cstdint>
#include "Color.hpp"
#include "Effect.hpp"

class Cave;

class Light : public Effect
{
	private:
		Color color;
	public:
		Color get_color() const { return color; }
		void set_color(const Color& color) { this->color = color; }

		Light();
		Light(const double radius, const size_t duration, const Color& color);
		Light(const Light& other);
		Light& operator=(const Light& other);

		void trigger(Cave& cave, const size_t idx) override;
};
