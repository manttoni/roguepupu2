#pragma once

#include <string>
#include "Effect.hpp"
#include "Utils.hpp"

class Cell;

class Entity
{
	private:
		std::string name;
	public:
		std::string get_name() const {
			return name;
		}
		void set_name(const std::string& name) {
			this->name = name;
		}

	private:
		short color_pair_id;
	public:
		auto get_color_pair_id() const {
			return color_pair_id;
		}
		void set_color_pair_id(const short cp_id) {
			color_pair_id = cp_id;
		}

	private:
		char ch;
	public:
		auto get_char() const {
			return ch;
		}
		void set_char(const char ch) {
			this->ch = ch;
		}

	private:
		Cell* cell;
	public:
		auto get_cell() { return cell; }
		void set_cell(Cell* cell) { this->cell = cell; }
		size_t get_idx() const;

	private:
		std::vector<Effect> effects; // f.e. blue glow from a fungus
	public:
		auto get_effects() const {
			return effects;
		}
		void set_effects(const std::vector<Effect>& effects) {
			this->effects = effects;
		}
		void add_effect(const Effect& effect) {
			effects.push_back(effect);
		}

	public:
		bool blocks_movement() const;
		bool blocks_vision() const;

	public:
		Entity();
		virtual ~Entity() = default;
		Entity(const std::string& name, const short color_pair_id, const char ch, Cell* cell);
		double move(const Direction d);
		bool operator==(const Entity& other);
};
