#pragma once

#include "LiquidMixture.hpp"
#include "Color.hpp"  // for Color
#include "entt.hpp"   // for size_t, map, entity, vector

class Cave;
class ColorPair;
class Cell
{
	private:
		size_t idx;
	public:
		size_t get_idx() const {
			return idx;
		}
		void set_idx(const size_t i) {
			this->idx = i;
		}
		bool is_border() const;

	public:
		enum class Type
		{
			None,	// default
			Rock,	// solid rock
			Floor,	// empty space with a floor
			Source,	// previous level
			Sink,	// next level
		};
	private:
		Type type;
	public:
		Type get_type() const {
			return type;
		}
		void set_type(const Cell::Type type) {
			this->type = type;
		}

	private:
		Cave* cave;
	public:
		Cave* get_cave() {
			return cave;
		}
		void set_cave(Cave* cave) {
			this->cave = cave;
		}

	private:
		wchar_t glyph;
	public:
		wchar_t get_glyph() const;
		void set_glyph(const wchar_t glyph) { this->glyph = glyph; }

	private:
		double density;
	public:
		void set_density(const double d) {
			this->density = d;
		}
		double get_density() const {
			return density;
		}
		void reduce_density(const double amount);

	private:
		Color fgcolor, bgcolor;
	public:
		Color get_fgcolor() const;
		Color get_bgcolor() const;
		void set_fgcolor(const Color& fg) { this->fgcolor = fg; }
		void set_bgcolor(const Color& bg) { this->bgcolor = bg; }
		ColorPair get_color_pair() const;

	private:
		std::map<Color, size_t> lights;
	public:
		auto get_lights() const {
			return lights;
		}
		void add_light(const Color& color) {
			lights[color]++;
		}
		void clear_lights() {
			lights.clear();
		}

	private:
		LiquidMixture liquid_mixture;
	public:
		double get_liquid_level() const;
		void clear_liquids() { liquid_mixture = LiquidMixture{}; }
		LiquidMixture& get_liquid_mixture() { return liquid_mixture; }
		const LiquidMixture& get_liquid_mixture() const { return liquid_mixture; }

	private:
		bool seen = false;
	public:
		bool is_seen() const { return seen; }
		void set_seen(const bool seen) { this->seen = seen; }
		bool has_landmark() const;
		wchar_t get_landmark_glyph() const;

	private:
		double humidity;
	public:
		double get_humidity() const { return humidity; }
		void set_humidity(const double value) { humidity = value; }

	public:
		Cell();
		Cell(const size_t idx, const Type& type, Cave* cave, const wchar_t symbol, const double density = 0);
		Cell(const Cell& other) = default;

		/* OVERLOADS */
		bool operator==(const Cell &other) const;
		bool operator!=(const Cell &other) const;
		bool operator<(const Cell &other) const;
		Cell& operator=(const Cell& other) = default;

		/* CELL FEATURES */
		bool blocks_vision() const;
		bool blocks_movement() const;
		bool is_empty() const;

		std::vector<entt::entity> get_entities() const;
		entt::registry& get_registry();
		const entt::registry& get_registry() const;
};
