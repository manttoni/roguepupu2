#pragma once

#include "game/entity/Definition.hpp"
#include "game/component/Base.hpp"
#include "external/entt/fwd.hpp"

namespace Game::Component::List
{
	template<typename T>
		struct Base : Game::Component::Base<T>
	{
		std::vector<T> values;

		Base() = default;

		explicit Base(std::vector<T> values)
			: values(std::move(values))
		{}

		[[nodiscard]] bool contains(const T& element) const
		{
			return std::find(
					values.begin(),
					values.end(),
					element
					) != values.end();
		}

		void remove(const T& element)
		{
			const auto it =
				std::find(values.begin(), values.end(), element);

			if (it != values.end())
				values.erase(it);
		}
	};

#define X(name, type)                                      \
	struct name : Game::Component::List::Base<type>              \
	{                                                       \
		using Game::Component::List::Base<type>::Base;            \
		static constexpr std::string_view string = #name;   \
	};

#include "List.def"
#undef X
}

