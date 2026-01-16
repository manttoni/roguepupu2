#include "external/entt.hpp"
#include "components/Components.hpp"
#include "utils/Log.hpp"
#include "domain/Conditions.hpp"
#include "utils/ECS.hpp"


namespace ConditionSystem
{
	bool is_true(const entt::registry& registry, const Conditions& conditions, const entt::entity entity)
	{
		Log::log("Checking conditions of " + ECS::get_name(registry, entity));

		/* Check weight */
		if (!registry.all_of<Weight>(entity))
		{
			if (conditions.weight_min || conditions.weight_max)
				return false;
		}
		const double weight = ECS::get_weight(registry, entity);
		if (conditions.weight_min && weight < *conditions.weight_min)
			return false;
		if (conditions.weight_max && weight > *conditions.weight_max)
			return false;

		/* Check category, subcategory counts as category */
		const auto& category = registry.get<Category>(entity).category;
		const auto& subcategory = registry.get<Subcategory>(entity).subcategory;
		if (conditions.category &&
				*conditions.category != category &&
				*conditions.category != subcategory)
		{
			return false;
		}
		if (conditions.category_not &&
				(*conditions.category_not == category ||
				 *conditions.category_not == subcategory))
		{
			return false;
		}

		return true;
	}
};
