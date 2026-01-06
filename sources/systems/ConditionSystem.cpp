#include "entt.hpp"
#include "Utils.hpp"
#include "Event.hpp"
#include "ECS.hpp"
#include "Components.hpp"

namespace ConditionSystem
{
	bool is_true(const entt::registry& registry, const Conditions& conditions, const entt::entity entity)
	{
		Log::log("Checking conditions of " + ECS::get_name(registry, entity));
		const double weight = ECS::get_weight(registry, entity);
		if (conditions.weight_min && weight < *conditions.weight_min)
			return false;
		if (conditions.weight_max && weight > *conditions.weight_max)
			return false;

		if (!registry.all_of<Category, Subcategory>(entity))
			Log::error("Entity doesnt have category");
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

		if (registry.all_of<Resources>(entity))
		{
			const auto hp = registry.get<Resources>(entity).health;
			const auto max_hp = ECS::get_health_max(registry, entity);
			const double per = static_cast<double>(hp) / static_cast<double>(max_hp);
			if (conditions.hp_min && per < *conditions.hp_min)
				return false;
			if (conditions.hp_max && per > *conditions.hp_max)
				return false;
		}
		return true;
	}
};
