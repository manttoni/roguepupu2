#include "utils/JsonUtils.hpp"

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
};
