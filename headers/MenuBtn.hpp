#pragma once
#include <string>
#include "MenuElt.hpp"

class MenuBtn : public MenuElt
{
	private:
		void (*func)();

	public:
		MenuBtn();
		MenuBtn(const std::string& text, void (*func)());
		~MenuBtn();

		void callback() const override { func(); }

		//std::string get_text() const override;
		size_t get_size() const override;
		std::any get_value() const override { return {}; }
};
