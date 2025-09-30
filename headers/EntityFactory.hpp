#pragma once

#include <memory>


class Fungus;
class Creature;
class Entity;
class EntityFactory
{
	public:
		static EntityFactory& instance()
		{
			static EntityFactory inst;
			return inst;
		}

	public:
		std::unique_ptr<Fungus> get_fungus(const std::string& name) const;
		std::unique_ptr<Creature> get_creature(const std::string& name) const;
		std::unique_ptr<Entity> get_entity(const std::string& name) const; // checks all previous
};
