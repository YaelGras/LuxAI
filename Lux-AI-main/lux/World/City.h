#pragma once

#include "Tiles.h"
#include <unordered_map>
#include <map>
class City {

private:

	std::map<iVec2, CityTile*> m_tiles{};
	std::string m_id;
	float m_fuelAmount;
	float m_fuelConsumption;

public:

	City() : m_id("")
		, m_fuelAmount(0)
		, m_fuelConsumption(0)
	{}

	City(const std::string& id, float fuelAmount, float consumption)
		: m_id(id)
		, m_fuelAmount(fuelAmount)
		, m_fuelConsumption(consumption)
	{
	}

	void update(float fuelAmount, float consumption) noexcept
	{
		m_fuelAmount = fuelAmount;
		m_fuelConsumption = consumption;
	}

	void attachCityTile(CityTile& city)
	{
		m_tiles[city.tilePos] = (&city);		
	}

	[[nodiscard]] size_t		size()							const noexcept { return m_tiles.size(); }
	[[nodiscard]] std::string	getId()							const noexcept { return m_id; }
	[[nodiscard]] float			getFuelAmount()					const noexcept { return m_fuelAmount; }
	[[nodiscard]] const std::map<iVec2, CityTile*>& getTiles()	const noexcept { return m_tiles; }
	[[nodiscard]] bool 			contains(iVec2 tile)			const noexcept {  return m_tiles.contains(tile); }


	void addFuel(float value_fuel) noexcept { m_fuelAmount += value_fuel; }

	float getConsumption() const { 
		return m_fuelConsumption;
	}

	bool hasEnoughFuel() const noexcept { return getFuelNeeded() > 0; }

	float getFuelNeeded() const noexcept {
	return std::max(getConsumption() * 10.f - m_fuelAmount, 0.f); }
};
