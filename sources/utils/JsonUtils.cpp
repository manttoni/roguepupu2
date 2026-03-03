#include "utils/JsonUtils.hpp"
#include "utils/Error.hpp"

namespace JsonUtils
{
	bool contains_all(const Json& super, const Json& sub)
	{
		if (super.type() != sub.type())
			return false;
		if (super == sub)
			return true;
		if (super.is_array())
		{
			for (const auto& elt : sub)
			{
				auto it = std::find(super.begin(), super.end(), elt);
				if (it == super.end())
					return false;
			}
		}
		else if (super.is_object())
		{
			for (const auto& [key, value] : sub.items())
			{
				if (!super.contains(key))
					return false;
				if (!contains_all(super.at(key), value))
					return false;
			}
		}
		return true;
	}
	bool contains_any(const Json& super, const Json& sub)
	{
		if (super.type() != sub.type())
			return false;
		if (super == sub)
			return true;
		if (super.is_array())
		{
			for (const auto& elt : sub)
			{
				auto it = std::find(super.begin(), super.end(), elt);
				if (it != super.end())
					return true;
			}
		}
		else if (super.is_object())
		{
			for (const auto& [key, value] : sub.items())
			{
				if (super.contains(key) && contains_any(super.at(key), value))
					return true;
			}
		}
		return false;
	}
	bool contains_none(const Json& super, const Json& sub)
	{
		return !contains_any(super, sub);
	}

	std::string to_string(const Json& j)
	{
		if (j.is_string())
			return j.get<std::string>();
		if (j.is_number_integer())
			return std::to_string(j.get<long long>());
		if (j.is_number_unsigned())
			return std::to_string(j.get<unsigned long long>());
		if (j.is_number_float())
			return std::to_string(j.get<double>());
		if (j.is_boolean())
			return j.get<bool>() ? "true" : "false";
		Error::fatal("Unhandled json type: " + j.dump(4));
	}

	/*Json merge(const Json& a, const Json& b, const bool replace)
	{
		if (a.type() != b.type())
			return Json{};

		if (a.is_object())
			return merge_objects(a, b, replace);
		else if (a.is_array())
			return merge_arrays(a, b, replace);
		else
			return Json{};
	}*/
};
