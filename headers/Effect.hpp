#pragma once

#include "Color.hpp"
#include "Cave.hpp"

class Effect
{
	public:
		enum class Type
		{
			NONE,
			GLOW
		};

	private:
		Type type;
		Color color;
		size_t duration;
		double radius;

		Type get_type() const { return type; }
		Color get_color() const { return color; }
		size_t get_duration() const { return duration; }
		double get_radius() const { return radius; }

		void set_type(const Type type) { this->type = type; }
		void set_color(const Color& color) { this->color = color; }
		void set_duration(const size_t duration) { this->duration = duration; }
		void set_radius(const double radius) { this->radius = radius; }

		Effect();
		Effect(const Type type, const Color& color, const size_t duration = SIZE_MAX, const double radius = 0.0);

		std::vector<const Cell*> affected_cells(const Cave& cave, const size_t middle);
		void trigger(Cave& cave, const size_t idx);

};
