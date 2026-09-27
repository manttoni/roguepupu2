#include "application/EntityDescriber.hpp"
#include "external/entt/entt.hpp"
#include <string>
#include "game/components/Component.hpp"

std::string EntityDescriber::describe(entt::entity entity) const
{
	if (entity == entt::null)
		return "<null entity>";

	const auto id = entt::to_integral(entity);

	if (!registry.valid(entity))
		return "<invalid entity " + std::to_string(id) + ">";

	if (const auto* name = registry.try_get<Component::Value::Name>(entity))
		return name->value;

	return "<unnamed entity " + std::to_string(id) + ">";
}
