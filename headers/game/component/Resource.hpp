#pragma once

#include <ostream>
#include <string_view>

#include "game/component/Base.hpp"

namespace Game::Component::Resource
{
	template<typename T>
		struct Base : Game::Component::Base<T>
	{
		T current{};
		T maximum{};

		Base() = default;
		explicit Base(const T& value)
			: current(value), maximum(value) {}

		void reset() { current = maximum; }
	};

#define X(name, type) \
	struct name : Game::Component::Resource::Base<type> \
	{ \
		using Base<type>::Base; \
		static constexpr std::string_view string = #name; \
		friend std::ostream& operator<<(std::ostream& os, const name& component) \
		{ \
			return os << name::string << ": " \
			<< component.current << "/" << component.maximum; \
		} \
	};

#include "Resource.def"
#undef X
}

