#pragma once
#include <ostream>
#include <string_view>

#include "game/component/Base.hpp"
#include "game/Enum.hpp"
#include "ncurses/Color.hpp"
#include "game/world/Position.hpp"
#include "game/component/Base.hpp"

namespace Game::Component::Value
{
	template<typename T>
		struct Base : Game::Component::Base<T>
	{
		T value{};

		Base() = default;
		explicit Base(const T& value) : value(value) {}
	};

#define X(name, type) \
	struct name : Game::Component::Value::Base<type> \
	{ \
		using Base<type>::Base; \
		static constexpr std::string_view string = #name; \
		bool operator==(const name& other) const \
		{ \
			return this->value == other.value; \
		} \
		friend std::ostream& operator<<(std::ostream& os, const name& component) \
		{ \
			os << name::string << ": "; \
			return os << component.value; \
		} \
	};

#include "Value.def"

#undef X
}

