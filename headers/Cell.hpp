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
		bool blocked;
		double density;
		short color_pair_id;
		std::map<short, size_t> glow; // key is id of color of glow, value is the amount of that glow from multiple sources
		std::vector<Entity> entities;
		char ch;

	public:
		/* CONSTRUCTORS */
		Cell();
		Cell(const size_t idx, const Type& type, const double density = 0);
		Cell(const Cell &other);

		/* GETTERS */
		size_t get_idx() const { return idx; }
		bool is_blocked() const { return blocked; }
		Type get_type() const { return type; }
		double get_density() const { return density; }
		short get_color_pair_id() const;
		char get_char() const;

		/* SETTERS */
		void set_type(const Cell::Type type) { this->type = type; }
		void set_blocked(const bool b) { this->blocked = b; }
		void set_density(const double d) { this->density = d; }
		void set_idx(const size_t i) { this->idx = i; }
		void set_char(const char ch) { this->ch = ch; }
		void set_color_pair_id(const short color_pair_id) { this->color_pair_id = color_pair_id; }

		/* LIGHTS */
		void reset_glow() { glow.clear(); }
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
};
