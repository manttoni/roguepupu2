#pragma once

#include <memory>
#include <string>
#include <vector>
#include <map>
#include "Entity.hpp"
#include "Utils.hpp"

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
		Cave* cave;
		double density;
		short color_pair_id;
		std::map<short, size_t> lights; // color_id : stacks
		std::vector<std::unique_ptr<Entity>> entities;

	public:
		/* CONSTRUCTORS */
		Cell();
		Cell(const size_t idx, const Type& type, Cave* cave, const double density = 0);
		Cell(Cell&& other);

		/* GETTERS */
		size_t get_idx() const {
			return idx;
		}
		Type get_type() const {
			return type;
		}
		double get_density() const {
			return density;
		}
		short get_color_pair_id() const;
		char get_char() const;
		auto get_lights() const {
			return lights;
		}
		Cave* get_cave() {
			return cave;
		}

		/* SETTERS */
		void set_type(const Cell::Type type) {
			this->type = type;
		}
		void set_density(const double d) {
			this->density = d;
		}
		void set_idx(const size_t i) {
			this->idx = i;
		}
		void set_color_pair_id(const short color_pair_id) {
			this->color_pair_id = color_pair_id;
		}
		void set_cave(Cave* cave) {
			this->cave = cave;
		}

		/* LIGHTS */
		void reset_effects();
		void add_light(const short light_id);

		/* ENTITIES */
		void add_entity(std::unique_ptr<Entity>&& ent) {
			entities.push_back(std::move(ent));
		}
		const auto& get_entities() const {
			return entities;
		}
		auto& get_entities() {
			return entities;
		}
		void move_entity(const Direction d);

		/* OVERLOADS */
		bool operator==(const Cell &other) const;
		bool operator!=(const Cell &other) const;
		bool operator<(const Cell &other) const;
		Cell &operator=(Cell&& other);

		/* MODIFY CELL */
		void reduce_density(const double amount);

		/* CELL FEATURES */
		bool blocks_vision() const;
		bool blocks_movement() const;
};
