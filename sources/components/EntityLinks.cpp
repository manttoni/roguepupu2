#include "components/EntityLinks.hpp"
#include "external/entt/entt.hpp"

void EntityLinks::add_link(const entt::entity entity)
{
	entities.push_back(entity);
}

void EntityLinks::remove_link(const entt::entity entity)
{
	auto it = std::find(entities.begin(), entities.end(), entity);
	if (it == entities.end())
		return;
	entities.erase(it);
}

bool EntityLinks::empty() const
{
	return entities.empty();
}

std::ostream& operator<<(std::ostream& os, const EntityLinks& el)
{
	os << "[";
	for (size_t i = 0; i < el.entities.size(); ++i)
	{
		if (i != 0)
			os << ", ";
		os << static_cast<size_t>(el.entities[i]);
	}
	os << "]";
	return os;
}
