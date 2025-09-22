#pragma once

#include <string>
#include "Light.hpp"
#include "Utils.hpp"
#include "Color.hpp"

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
		Color color;
	public:
		Color get_color() const { return color; }
		void set_color(const Color& color) { this->color = color; }

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
		std::vector<Light> lights; // f.e. blue glow from a fungus
	public:
		auto get_lights() const {
			return lights;
		}
		void set_lights(const std::vector<Light>& lights) {
			this->lights = lights;
		}
		void add_light(const Light& light) {
			lights.push_back(light);
		}



	public:
		bool blocks_movement() const;
		bool blocks_vision() const;

	public:
		Entity();
		virtual ~Entity() = default;
		Entity(const std::string& name, const Color& color, const char ch, Cell* cell);
		double move(const Direction d);
		bool operator==(const Entity& other);
};
