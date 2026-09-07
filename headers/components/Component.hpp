#pragma once

#include <concepts>
#include <cstddef>
#include <ostream>
#include <string_view>
#include <type_traits>
#include <vector>
#include "ncurses/Color.hpp"
#include "domain/Enum.hpp"
#include "external/entt/fwd.hpp"

namespace Component
{
	/* operator<< of char is deleted
	 * */
	template<typename T>
		std::ostream& print_value(std::ostream& os, const T& value)
		{
			if constexpr (std::same_as<T, char>)
			{
				return os << "U+" << static_cast<std::uint32_t>(value);
			}
			else
			{
				return os << value;
			}
		}
}

namespace Component::List
{
	template<typename T>
		struct Base
		{
			using value_type = T;
			using container_type = std::vector<T>;
			using iterator = container_type::iterator;
			using const_iterator = container_type::const_iterator;

			container_type values;

			iterator begin() noexcept { return values.begin(); }
			iterator end() noexcept { return values.end(); }

			const_iterator begin() const noexcept { return values.begin(); }
			const_iterator end() const noexcept { return values.end(); }

			const_iterator cbegin() const noexcept { return values.cbegin(); }
			const_iterator cend() const noexcept { return values.cend(); }

			[[nodiscard]] bool empty() const noexcept
			{
				return values.empty();
			}

			[[nodiscard]] std::size_t size() const noexcept
			{
				return values.size();
			}

			[[nodiscard]] bool contains(const T& element) const
			{
				auto it = std::find(values.begin(), values.end(), element);
				return it != values.end();
			}

			void remove(const T& element)
			{
				auto it = std::find(values.begin(), values.end(), element);
				if (it == values.end())
					return;
				values.erase(it);
			}

			void push_back(const T& element) { values.push_back(element); }
		};
#define X(name, type) \
	struct name : Base<type> \
	{ \
		static constexpr std::string_view string = #name; \
	};
#include "List.def"
#undef X
}



namespace Component::Value
{
	template<typename T>
		std::ostream& print_component_value(
				std::ostream& os,
				const T& value)
		{
			if constexpr (std::is_enum_v<T>)
				return os << Domain::Enum::to_string(value);
			else
				return print_value(os, value);
		}

	template<typename T>
		struct Base
		{
			using value_type = T;

			T value{};

			Base() = default;
			explicit Base(const T& value) : value(value) {}
		};

#define X(name, type)                                                   \
	struct name : Base<type>                                        \
	{                                                               \
		using Base<type>::Base;                                  \
		static constexpr std::string_view string = #name;         \
		friend std::ostream& operator<<(std::ostream& os, const name& component) \
		{                                                       \
			os << name::string << ": ";                       \
			return print_component_value(                    \
					os, component.value);             \
		}                                                       \
	};

#include "Value.def"

#undef X
}

namespace Component::Resource
{
	template<typename T>
		struct Base
		{
			using value_type = T;
			T current = T{};
			T maximum = T{};

			void reset() { current = maximum; }
			Base(const T& value) : current(value), maximum(value) {}
			Base() = default;
		};

#define X(name, type, ...) \
	struct name : Component::Resource::Base<type> \
	{ \
		static constexpr std::string_view string = #name; \
		friend std::ostream& operator<<(std::ostream& os, const name& component) \
		{ \
			os << name::string << ": "; \
			print_value(os, component.current); \
			os << "/"; \
			return print_value(os, component.maximum); \
		} \
		__VA_OPT__(name() : Resource::Base<type>(__VA_ARGS__) {}) \
	};
#include "Resource.def"
#undef X
}

namespace Component::Tag
{
	struct Base {};
#define X(name) \
	struct name : Component::Tag::Base \
	{ \
		static constexpr std::string_view string = #name; \
		friend std::ostream& operator<<(std::ostream& os, const name&) \
		{ \
			return os << name::string; \
		} \
	};
#include "Tag.def"
#undef X
}

namespace Component::Dependency
{
	template<typename C, typename D>
		constexpr bool is_dependent()
		{
			return false

#define X(component, dependency) \
				|| (std::same_as<C, component> && std::same_as<D, dependency>)
#include "Dependency.def"
#undef X

				;
		}
}
