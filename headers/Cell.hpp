#pragma once

#include <string>
#include <vector>
#include <map>
#include "Entity.hpp"

class Cell
{
	public:
		enum class Type
		{
			ROCK,	// solid rock
			FLOOR,	// empty space with a floor
			SOURCE,	// previous level
			SINK,	// next level
			NONE,	// can be anything
		};

	private:
		size_t idx;
		Type type;
		double density;
		short color_pair_id;
		std::map<short, size_t> glow; // key is id of color of glow, value is the amount of that glow from multiple sources
		std::vector<Entity> entities;

	public:
		/* CONSTRUCTORS */
		Cell();
		Cell(const size_t idx, const Type& type, const double density = 0);
		Cell(const Cell &other);

		/* GETTERS */
		size_t get_idx() const { return idx; }
		Type get_type() const { return type; }
		double get_density() const { return density; }
		short get_color_pair_id() const;
		char get_char() const;
		auto get_glow() const { return glow; }

		/* SETTERS */
		void set_type(const Cell::Type type) { this->type = type; }
		void set_density(const double d) { this->density = d; }
		void set_idx(const size_t i) { this->idx = i; }
		void set_color_pair_id(const short color_pair_id) { this->color_pair_id = color_pair_id; }

		/* LIGHTS */
		void reset_effects();
		void add_glow(const short glow_id);

		/* ENTITIES */
		void add_entity(const Entity& ent) { entities.push_back(ent); }
		auto get_entities() const { return entities; }

		/* OVERLOADS */
		bool operator==(const Cell &other) const;
		bool operator!=(const Cell &other) const;
		bool operator<(const Cell &other) const;
		Cell &operator=(const Cell &other);

		/* MODIFY CELL */
		void reduce_density(const double amount);

		/* CELL FEATURES */
		bool blocks_vision() const;
		bool blocks_movement() const;
};
