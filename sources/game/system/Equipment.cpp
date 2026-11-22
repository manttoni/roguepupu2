#include <vector>
#include "game/system/Equipment.hpp"
#include "game/component/Component.hpp"
#include "external/entt/entt.hpp"

namespace Game::System::Equipment
{
	bool is_equipped(
		const entt::registry& registry,
		const entt::entity entity,
		const entt::entity item)
	{
		if (!registry.all_of<
			Component::Tag::Item,
			Component::Tag::Equipment>(item))
			return false;

		const auto* equipped_items =
			registry.try_get<Component::List::EquippedItems>(entity);
		if (!equipped_items || equipped_items->values.empty())
			return false;

		for (const auto& equipped_item : equipped_items->values)
		{
			if (item == equipped_item)
				return true;
		}
		return false;
	}

	bool equip(
		entt::registry& registry,
		const entt::entity entity,
		const entt::entity item)
	{
		if (!registry.all_of<
			Component::Tag::Item,
			Component::Tag::Equipment>(item))
			throw std::runtime_error("Invalid item");

		const auto* inventory =
			registry.try_get<Component::List::Inventory>(entity);
		if (!inventory)
			throw std::runtime_error("Entity has no inventory");
		if (!inventory->contains(item))
			throw std::runtime_error("Can only equip items in inventory");

		auto* equipped_items =
			registry.try_get<Component::List::EquippedItems>(entity);
		if (!equipped_items)
			throw std::runtime_error("Entity cannot equip items");

		if (is_equipped(registry, entity, item))
			return false;

		equipped_items->values.push_back(item);
		return true;
	}

	bool unequip(
		entt::registry& registry,
		const entt::entity entity,
		const entt::entity item)
	{
		if (!registry.all_of<
			Component::Tag::Item,
			Component::Tag::Equipment>(item))
			return false;

		auto* equipped_items =
			registry.try_get<Component::List::EquippedItems>(entity);
		if (!equipped_items || equipped_items->values.empty())
			return false;

		if (!is_equipped(registry, entity, item))
			return false;

		equipped_items->remove(item);
		return true;
	}
}
