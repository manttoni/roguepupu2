#pragma once
#include <string>
#include <any>

class MenuElt
{
	protected:
		std::string text;

	private:
		std::string type;

	public:
		MenuElt();
		MenuElt(const std::string& text, const std::string& type);
		virtual ~MenuElt() = default;

		std::string get_type() const { return type; }

		virtual std::string get_text() const { return text; }
		virtual size_t get_size() const = 0;

		virtual void callback() const {}
		virtual void increment() {}
		virtual void decrement() {}
		virtual std::any get_value() const = 0;
};
