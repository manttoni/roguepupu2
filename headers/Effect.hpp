#pragma once

#include <vector>

class Cell;
class Cave;
class Effect
{
	public:
		enum class Type
		{
			NONE,
			LIGHT,
		};
	protected:
		Type type;
	public:
		Type get_type() const { return type; }
		void set_type(const Type type) { this->type = type; }

	protected:
		double radius;
	public:
		double get_radius() const { return radius; }
		void set_radius(const double radius) { this->radius = radius; }

	protected:
		size_t duration;
	public:
		size_t get_duration() const { return duration; }
		void set_duration(const size_t duration) { this->duration = duration; }

	public:
		Effect(const Type type, const double radius, const size_t duration);
		Effect();
		~Effect();
		Effect(const Effect& other);
		Effect& operator=(const Effect& other);

		virtual void trigger(Cave& cave, const size_t idx) = 0;
};
