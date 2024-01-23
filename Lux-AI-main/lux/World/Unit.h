#pragma once

#include <iostream>
#include <string>
#include <ranges>
#include <tuple>
#include <utility>
#include "Map.h"
#include "Tiles.h"

using RessourceCounts = std::tuple<int, int, int>;

constexpr int FUEL_WOOD = 1;
constexpr int FUEL_COAL = 10;
constexpr int FUEL_URANIUM = 40;

class Unit {


protected:

	iVec2		m_tilePos; // tilePose
	std::string m_uid;
	float		m_cooldown;

public:
	virtual ~Unit() = default;

	Unit()
		: m_tilePos({ -1,-1 })
		, m_uid("u_-1")
		, m_cooldown(-1)
	{}

	Unit(iVec2 p, const std::string& uid, float cd = 0)
		: m_tilePos(p)
		, m_uid(uid)
		, m_cooldown(cd)
	{}

	[[nodiscard]] virtual std::string	getId() const noexcept { return m_uid; }
	[[nodiscard]] virtual iVec2			getPos() const noexcept { return m_tilePos; }
	[[nodiscard]] virtual float			getCooldown() const noexcept { return m_cooldown; }
	[[nodiscard]] virtual bool			canAct() const noexcept { return m_cooldown < 1.f; }

	bool operator==(const Unit& other) const { return m_uid == other.m_uid; }
	bool operator<(const Unit& other) const { return std::stoi(m_uid.c_str() + 2) < std::stoi(other.m_uid.c_str() + 2); }

};

namespace std {
	template <>
	struct hash<Unit>
	{
		size_t operator()(const Unit& x) const noexcept
		{
			return std::stoi(x.getId().c_str() + 2);
		}
	};
}


class MovableUnit final : public Unit 
{
protected:

	RessourceCounts resources;
	enum class UnitType : int { WORKER, CART, UNDEFINED } m_type;
	int m_fuelConsumption;

public:

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	MovableUnit() 
		: Unit()
		, m_type(UnitType::UNDEFINED) 
		, m_fuelConsumption(INT_MAX)
	{}

	MovableUnit(const iVec2 p, const std::string& uid, float cd = 0, UnitType type= UnitType::UNDEFINED)
		: Unit(p,uid,cd)
		, m_type(type)
	{	m_fuelConsumption = (m_type == UnitType::WORKER) ? 4: 10; }


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// -- Setters / Getters
	
	void setPos(const iVec2 p)						noexcept { m_tilePos = p; }
	void setResources(const RessourceCounts res)	noexcept { resources = res; }

	[[nodiscard]] const RessourceCounts& getResources() const { return resources; }
	[[nodiscard]] int getResourceTotalCount() const noexcept {	return std::apply([](auto&&  ...p) {return (p + ...); }, resources); }

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// -- Methods

	[[nodiscard]] bool canBuild() const noexcept {	return getResourceTotalCount() >= 100; }	
	
	[[deprecated]] virtual std::string move() { return "NULL"; }
	[[deprecated]] virtual std::string transfer() { return "NULL"; }

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// -- Constructors


	~MovableUnit() override = default;
	MovableUnit(const MovableUnit&) = default;
	MovableUnit& operator=(const MovableUnit&) = default;
	MovableUnit(MovableUnit&&) = default;
	MovableUnit& operator=(MovableUnit&&) = default;

};

