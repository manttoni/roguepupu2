#pragma once

class Color
{
	private:
		short id;
		short r, g, b;

	public:
		short get_r() const { return r; }
		short get_g() const { return g; }
		short get_b() const { return b; }
		short get_id() const { return id; }

	public:
		Color();
		Color(const short id, const short r, const short g, const short b);
		Color(const Color& other);
		Color& operator=(const Color& other);
		bool operator==(const Color& other);
		bool operator!=(const Color& other);
};
