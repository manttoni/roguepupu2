#pragma once
#include <string>
#include <functional>
#include "MenuElt.hpp"

class MenuBtn : public MenuElt
{
	private:
		std::function<void()> func;

	public:
		MenuBtn();
		MenuBtn(const std::string& text, std::function<void()> f = nullptr);
		~MenuBtn();

		void callback() const override { if(func != nullptr) func(); }
		std::function<void()> get_func() const override { return func; }
		size_t get_size() const override;
		std::any get_value() const override { return {}; }
};
