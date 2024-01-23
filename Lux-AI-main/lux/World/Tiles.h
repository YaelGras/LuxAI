#pragma once

#include <iostream>
#include <string>
#include <variant>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct iVec2 {

	union {
		struct { int first; int second; };  // NOLINT(clang-diagnostic-nested-anon-types)
		struct { int x;		int y; };		// NOLINT(clang-diagnostic-nested-anon-types)
	};

	iVec2 operator+(const iVec2 other) const {	return {x + other.x, y + other.y}; }
	iVec2 operator-(const iVec2 other) const {	return {x - other.x, y - other.y}; }

	bool operator==(const iVec2& other) const {	return other.x == x && other.y == y;}
	bool operator<(const iVec2& other) const {	return std::tie(x,y) < std::tie(other.x,other.y);}

};

inline std::ostream& operator<<(std::ostream& os, const iVec2& v)
{
	os << "(" << v.x << "," << v.y << ")";
	return os;
}


namespace std {
	template <>
	struct hash<iVec2>
	{
		int w;
		size_t operator()(const iVec2& x) const noexcept
		{
			return x.first + x.second * w;
		}
	};
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///
/// -- Forward declarations

class City;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct Tile { iVec2 tilePos; };
struct OOBTile : Tile {};
struct EmptyTile : Tile {};

struct CityTile : public Tile {

	City* m_city = nullptr;
	float cooldown = 0;

	CityTile() = default;

	CityTile(City& city, const iVec2& pos, float cd)
		: Tile{ pos }
		, m_city(&city)
		, cooldown(cd)
	{}


	[[nodiscard]] bool canAct() const noexcept { return cooldown < 1.f; }
	[[nodiscard]] float getFuelConsumption() const noexcept;
};

struct EnemyCityTile : public Tile
{

	EnemyCityTile() = default;

	explicit EnemyCityTile(const iVec2& pos)
		: Tile{ pos }
	{}


};


struct ResourceTile : Tile {

	enum ResType { WOOD, COAL, URANIUM, INVALID };
	int amount;
	ResType type;

	ResourceTile() : Tile(), amount(0), type(INVALID) {}

	ResourceTile(
			const iVec2& pos,
			const std::string& type_str,
			int amount = 0)
		: Tile{ pos }
		, amount(amount)
		, type(INVALID)
	{
		if (type_str == "wood")
			type = WOOD;
		if (type_str == "coal")
			type = COAL;
		if (type_str == "uranium")
			type = URANIUM;
	}
};

using Road = float;
using varTile = std::variant<Tile, ResourceTile, CityTile, OOBTile, EmptyTile>;