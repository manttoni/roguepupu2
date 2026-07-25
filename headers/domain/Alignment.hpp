#include <unordered_map>
#include <string>
#include <cmath>

struct Alignment
{
	enum class Type
	{
		LawfulGood,
		LawfulNeutral,
		LawfulEvil,
		NeutralGood,
		TrueNeutral,
		NeutralEvil,
		ChaoticGood,
		ChaoticNeutral,
		ChaoticEvil
	};
	double chaos_law = 0.0;
	double evil_good = 0.0;
	double tolerance = 0.0;
	std::unordered_map<entt::entity, double> personal_opinions;

	Alignment() = default;
	Alignment(const double chaos_law, const double evil_good, const double tolerance = 0.0)
		: chaos_law(chaos_law), evil_good(evil_good), tolerance(tolerance) {}
	Alignment(const Type type, const double tolerance = 0.0) : tolerance(tolerance)
	{
		switch (type)
		{
			case Type::LawfulGood: chaos_law = 1; evil_good = 1; break;
			case Type::LawfulNeutral: chaos_law = 1; evil_good = 0; break;
			case Type::LawfulEvil: chaos_law = 1; evil_good = -1; break;
			case Type::NeutralGood: chaos_law = 0; evil_good = 1; break;
			case Type::TrueNeutral: chaos_law = 0; evil_good = 0; break;
			case Type::NeutralEvil: chaos_law = 0; evil_good = -1; break;
			case Type::ChaoticGood: chaos_law = -1; evil_good = 1; break;
			case Type::ChaoticNeutral: chaos_law = -1; evil_good = 0; break;
			case Type::ChaoticEvil: chaos_law = -1; evil_good = -1; break;
		}
	}

	std::string to_string() const
	{
		std::string ret = "";
		if (chaos_law <= -0.5) ret = "Chaotic ";
		else if (chaos_law < 0.5) ret = "Neutral ";
		else ret = "Lawful ";

		if (evil_good <= -0.5) ret += "Evil";
		else if (evil_good < 0.5) ret += "Neutral";
		else ret += "Good";

		if (ret == "Neutral Neutral") return "True Neutral";
		return ret;
	}

	double distance(const Alignment& other) const
	{
		return hypot(chaos_law - other.chaos_law, evil_good - other.evil_good);
	}
};
