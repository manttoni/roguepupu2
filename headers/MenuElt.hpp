#pragma once
#include <string>
#include <any>

class MenuElt
{
	public:
		enum class Type
		{
			TEXT,
			NUMBER,
			BUTTON,
			NONE,
		};
	protected:
		std::string text;

	private:
		Type type;

	public:
		MenuElt();
		MenuElt(const std::string& text, const Type type = Type::NONE);
		virtual ~MenuElt() = default;


		Type get_type() const { return type; }
		bool is_selectable() const { return type == Type::NUMBER || type == Type::BUTTON; }

		virtual std::string get_text() const { return text; }
		virtual size_t get_size() const = 0;

		virtual void callback() const {}
		virtual void increment() {}
		virtual void decrement() {}
		virtual std::any get_value() const = 0;
		virtual void set_value(std::any value) {(void) value;};
		virtual void (*get_func() const)() = 0;
};
