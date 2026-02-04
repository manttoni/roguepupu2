#include <string>
#include <optional>

#include "infrastructure/CharacterCreation.hpp"
#include "external/entt/entt.hpp"
#include "UI/Menu.hpp"
#include "components/Components.hpp"
#include "utils/Screen.hpp"
#include "systems/state/StateSystem.hpp"
#include "domain/Color.hpp"
#include "external/entt/entity/fwd.hpp"

namespace CharacterCreation
{
	using Element = Menu::Element;
	using Type = Element::Type;

	entt::entity create_character(entt::registry& registry)
	{

		Menu cc(Screen::middle());
		cc.add_element(Element(Type::Text, "*** Character Creation/Menu test ***"));
		cc.add_element(Element(Type::Text, "--"));

		Element name(Type::TextField, "Name");
		name.min_input = 1;
		name.input = "Rabdin";
		cc.add_element(name);

		Element glyph(Type::TextField, "Glyph");
		glyph.max_input = 1;
		glyph.min_input = 1;
		glyph.input = "@";
		cc.add_element(glyph);

		cc.add_element(Element(Type::ValueSelector, "Endurance", 10, 20));
		cc.add_element(Element(Type::ValueSelector, "Willpower", 10, 20));
		cc.add_element(Element(Type::ValueSelector, "Vitality", 10, 20));
		cc.add_element(Element(Type::ValueSelector, "Perception", 10, 2000));
		cc.add_element(Element(Type::ValueSelector, "Charisma", 10, 20));

		cc.add_element(Element(Type::ValueSelector, "Chaos - Law", -1, 1));
		cc.add_element(Element(Type::ValueSelector, "Evil - Good", -1, 1));

		cc.add_element(Element(Type::Checkbox, "Testbox", true));

		cc.add_element(Element(Type::Button, "Ready"));

		auto selection = cc.get_selection();
		if (selection.label == "")
			return entt::null;

		entt::entity character = registry.create();
		registry.emplace<Player>(character);
		registry.emplace<FGColor>(character, Color(123,456,789));
		registry.emplace<Solid>(character);
		registry.emplace<Weight>(character, 70.0);
		registry.emplace<Size>(character, 70.0);
		registry.emplace<EquipmentSlots>(character);
		registry.emplace<Inventory>(character);
		registry.emplace<Category>(character, "creatures");
		registry.emplace<Subcategory>(character, "players");
		registry.emplace<Name>(character, cc.get_input("Name"));
		registry.emplace<Glyph>(character, cc.get_input("Glyph").front());
		registry.emplace<Endurance>(character, cc.get_value("Endurance"));
		registry.emplace<Stamina>(character, StateSystem::get_max_stamina(registry, character));
		registry.emplace<Willpower>(character, cc.get_value("Willpower"));
		registry.emplace<Mana>(character, StateSystem::get_max_mana(registry, character));
		registry.emplace<Vitality>(character, cc.get_value("Vitality"));
		registry.emplace<Health>(character, StateSystem::get_max_health(registry, character));
		registry.emplace<Perception>(character, cc.get_value("Perception"));
		registry.emplace<Charisma>(character, cc.get_value("Charisma"));
		registry.emplace<Alignment>(character,
				static_cast<double>(cc.get_value("Chaos - Law")),
				static_cast<double>(cc.get_value("Evil - Good")));

		return character;
	}
};
