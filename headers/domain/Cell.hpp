#pragma once

#define CELL_DENSITY_MAX 9

#include <limits>
#include "LiquidMixture.hpp"
#include "Color.hpp"  // for Color

class Cell
{
	private:
		size_t idx;
	public:
		size_t get_idx() const { return idx; }
		void set_idx(const size_t idx) { this->idx = idx; }

	public:
		enum class Type
		{
			None,
			Rock,
			Floor,
			Source,
			Sink
		};
	public:
		Type get_type() const
		{
			if (density == std::numeric_limits<double>::infinity())
				return Type::Source;
			if (density == -std::numeric_limits<double>::infinity())
				return Type::Sink;
			return density > 0 ? Type::Rock : Type::Floor;
		}

	private:
		Color fgcolor, bgcolor;
	public:
		Color get_fgcolor() const { return fgcolor; }
		Color get_bgcolor() const { return bgcolor; }

	private:
		wchar_t glyph;
	public:
		wchar_t get_glyph() const;
		void set_glyph(const wchar_t glyph) { this->glyph = glyph; }

	private:
		double density;
	public:
		void set_density(const double d) { this->density = d; }
		double get_density() const { return density; }
		void reduce_density(const double amount);

	private:
		std::map<Color, size_t> lights;
	public:
		auto get_lights() const { return lights; }
		void add_light(const Color& color) { lights[color]++; }
		void clear_lights() { lights.clear(); }

	private:
		// what liquids are flowing into the cave from this source
		// not used for non-source cells unless some condensation effect is implemented
		LiquidMixture liquid_source;
	public:
		void set_liquid_source(const LiquidMixture& liquid_source)
		{
			assert(get_type() == Type::Source);
			this->liquid_source = liquid_source;
		}
		LiquidMixture get_liquid_source() const
		{
			assert(get_type() == Type::Source);
			return liquid_source;
		}

	private:
		// what liquids are now in this cell
		LiquidMixture liquid_mixture;
	public:
		double get_liquid_level() const;
		void clear_liquids() { liquid_mixture = LiquidMixture{}; }
		LiquidMixture& get_liquid_mixture() { return liquid_mixture; }
		const LiquidMixture& get_liquid_mixture() const { return liquid_mixture; }

	public:
		Cell(const size_t idx, const Cell::Type type = Cell::Type::Rock);

		bool operator==(const Cell &other) const = default;
		bool operator!=(const Cell &other) const = default;
		Cell& operator=(const Cell& other) = default;

};
