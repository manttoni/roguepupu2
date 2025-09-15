#pragma once

#include <vector>
#include <cstdint>

class Cave;

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
		short color_id;
		double radius;

	public:
		Type get_type() const { return type; }
		short get_color_id() const { return color_id; }
		double get_radius() const { return radius; }

		void set_type(const Type type) { this->type = type; }
		void set_color(const short color_id) { this->color_id = color_id; }
		void set_radius(const double radius) { this->radius = radius; }

		Effect();
		Effect(const Type type, const short color_id, const double radius = 0.0);

		std::vector<size_t> affected_cells_ids(const Cave& cave, const size_t middle) const;
		void trigger(Cave& cave, const size_t idx);

};
