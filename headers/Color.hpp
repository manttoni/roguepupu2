#pragma once

class Color
{
	private:
		short r, g, b;
		short id;

	public:
		Color();
		Color(const short r, const short g, const short b);
		Color(const Color& other);
		Color& operator=(const Color& other);
		bool operator==(const Color& other);
		bool operator!=(const Color& other);
		Color operator+(const Color& other) const;
		Color operator-(const Color& other) const;
		short get_id() const { return id; }
};
