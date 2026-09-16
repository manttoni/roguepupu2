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
	template<typename T>
		std::ostream& print_value(std::ostream& os, const T& value)
		{
			return os << value;
		}

	template<typename T>
		struct Base
		{
			using value_type = T;
		};

	template<typename T>
		std::vector<std::string> enum_value_strings()
		{
			if constexpr (Domain::Enum::GameEnum<T>)
			{
				return Domain::Enum::get_value_strings<T>();
			}
			else
			{
				return {};
			}
		}

	inline std::vector<std::string> get_enum_value_strings(const std::string_view component_id)
	{

#define X(name, type) \
		if (component_id == #name) \
		{ \
			return enum_value_strings<type>(); \
		}

#include "List.def"
#include "Value.def"
#include "Resource.def"

#undef X

		return {};
	}

	static inline bool value_is_enum(const std::string_view& component_id)
	{
		if (component_id.empty())
			return false;

#define X(name, type) \
		else if (component_id == #name) \
		{ \
			return Domain::Enum::GameEnum<type>; \
		}

#include "List.def"
#include "Value.def"
#include "Resource.def"

#undef X
		return false; // Could still be some other enum than GameEnum
	}
}

namespace Component::List
{
	template<typename T>
		struct Base : Component::Base<T>
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
	struct name : Component::List::Base<type>              \
	{                                                       \
		using Component::List::Base<type>::Base;            \
		static constexpr std::string_view string = #name;   \
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
			if constexpr (Domain::Enum::GameEnum<T>)
				return os << Domain::Enum::to_string(value);
			else
				return print_value(os, value);
		}

	template<typename T>
		struct Base : Component::Base<T>
	{
		T value{};

		Base() = default;
		explicit Base(const T& value) : value(value) {}
	};

#define X(name, type) \
	struct name : Component::Value::Base<type> \
	{ \
		using Base<type>::Base; \
		static constexpr std::string_view string = #name; \
		friend std::ostream& operator<<(std::ostream& os, const name& component) \
		{ \
			os << name::string << ": "; \
			return print_component_value( \
					os, component.value); \
		} \
	};

#include "Value.def"

#undef X
}

namespace Component::Resource
{
	template<typename T>
		struct Base : Component::Base<T>
	{
		T current = T{};
		T maximum = T{};

		void reset() { current = maximum; }
		Base(const T& value) : current(value), maximum(value) {}
		Base() = default;
	};

#define X(name, type) \
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
	template<typename C, typename D> constexpr bool is_dependent()
	{
		return false

#define X(component, dependency) \
			|| (std::same_as<C, component> && std::same_as<D, dependency>)
#include "Dependency.def"
#undef X

			;
	}

	// get ids of components that are dependent on C
	inline std::vector<std::string>
		get_directly_dependent_tag_ids(
				const std::string_view dependency_id)
		{
			static constexpr std::string_view prefix =
				"Component::Tag::";

			std::vector<std::string> tag_ids;

#define X(component, dependency) \
			{ \
				constexpr std::string_view component_id{#component};\
				constexpr std::string_view required_id{#dependency};\
				if (component_id.starts_with(prefix) && \
						required_id.starts_with(prefix) && \
						required_id.substr(prefix.size()) == \
						dependency_id) \
				{ \
					tag_ids.emplace_back( \
							component_id.substr(prefix.size())); \
				} \
			}

#include "Dependency.def"

#undef X

			return tag_ids;
		}
}
