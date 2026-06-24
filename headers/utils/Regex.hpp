#include <regex>

namespace Regex
{
	constexpr std::string_view DICE_ROLL = R"(^(\d+)d(\d+)([+-]\d+)?$)";
};
