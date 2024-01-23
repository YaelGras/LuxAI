#pragma once

#include <algorithm>
#include <string>
#include <vector>
#include <optional>
#include <numeric>
#include <ranges>

#include "City.h"
#include "Tiles.h"
#include <unordered_map>

#include "lux/Utility/Clustering.h"

template <typename T> 
int sgn(T val) {
	return (T(0) < val) - (val < T(0));
}

enum class DIRECTIONS : char
{
	NORTH = 'n',
	EAST = 'e',
	SOUTH = 's',
	WEST = 'w',
	CENTER = 'c'
};
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Map {


private:

	int width{}, height{};
																		/*	Used for iVec2 power hashing	*/
	std::unordered_map<iVec2, CityTile>		m_cityTiles					{ static_cast<unsigned int> (width) };
	std::unordered_map<iVec2, EnemyCityTile>m_enemyCities				{ static_cast<unsigned int> (width) }; // could be a set ?
	std::unordered_map<iVec2, ResourceTile>	m_AccessibleResourceTiles	{ static_cast<unsigned int> (width) };
	std::unordered_map<iVec2, ResourceTile>	m_AllResourceTiles			{ static_cast<unsigned int> (width) };
	std::unordered_map<iVec2, Road>			m_roads						{ static_cast<unsigned int> (width) };
	std::unordered_map<std::string, City>	m_allCities					{ static_cast<unsigned int> (width) };

	std::unordered_map<int, Cluster> m_clusters;
	std::vector<int> m_accessibleClusters;
	
public:

	[[nodiscard]] iVec2			getSize()						const noexcept { return {width, height}; }
	[[nodiscard]] float			getRoadValue(iVec2 tile)		const noexcept { return m_roads.at(tile); }
	[[nodiscard]] ResourceTile	getResourceTile(iVec2 tile)		const noexcept { return m_AccessibleResourceTiles.at(tile); }
	[[nodiscard]] ResourceTile	getALLResourceTile(iVec2 tile)	const noexcept { return m_AllResourceTiles.at(tile); }
	[[nodiscard]] CityTile		getCityTile(iVec2 tile)			const noexcept { return m_cityTiles.at(tile); }
	[[nodiscard]] CityTile&		getCityTile(iVec2 tile)			noexcept { return m_cityTiles.at(tile); }
	[[nodiscard]] City			getCity(const std::string& id)	const noexcept { return m_allCities.at(id);	}
	[[nodiscard]] City&			getCity(const std::string& id)	noexcept { return m_allCities.at(id);	}

	[[nodiscard]] const std::unordered_map<iVec2, CityTile>&		getAllCityTiles()		const noexcept { return m_cityTiles; }
	[[nodiscard]] const std::unordered_map<iVec2, EnemyCityTile>&	getEnemyCityTiles()		const noexcept { return m_enemyCities; }
	[[nodiscard]] const std::unordered_map<iVec2, ResourceTile>&	getAccessibleResTiles()	const noexcept { return m_AccessibleResourceTiles; }
	[[nodiscard]] const std::unordered_map<iVec2, ResourceTile>&	getAllResTiles()		const noexcept { return m_AllResourceTiles; }
	[[nodiscard]] const std::unordered_map<iVec2, Road>&			getAllRoads()			const noexcept { return m_roads; }
	[[nodiscard]] const std::unordered_map<std::string, City>&		getAllCities()			const noexcept { return m_allCities; }
	[[nodiscard]] const std::unordered_map<int, Cluster>&			getClusters()			const noexcept { return m_clusters; }
	[[nodiscard]] std::vector<Cluster>								getClustersAsVector()	const noexcept
	{
		std::vector<Cluster> res;
		for (const auto& c : m_clusters | std::views::values)
		{
			res.emplace_back(c);

		}
		return res;
	}

	void updateTile(const CityTile& tileInfos)				{ m_cityTiles[tileInfos.tilePos] = tileInfos; }
	void updateEnnemyCityTile(EnemyCityTile tile)	{ m_enemyCities[tile.tilePos] = tile; }
	void updateTile(ResourceTile tileInfos)			{ m_AccessibleResourceTiles[tileInfos.tilePos] = tileInfos; }
	void updateALLTile (ResourceTile tileInfos)		{ m_AllResourceTiles[tileInfos.tilePos] = tileInfos; }
	void updateRoad(iVec2 tile, float roadValue)	{ m_roads[tile] = roadValue; }
	void addCity(City&& city) noexcept				{ m_allCities.insert({ city.getId(), city }); }
	void setSize(const iVec2& size) noexcept { width = size.x; height = size.y; }

	[[nodiscard]] int getResourceCount(ResourceTile::ResType Type) const {
		return std::accumulate(
			m_AllResourceTiles.begin(), m_AllResourceTiles.end(),
			0,
			[&Type](int acc, const std::pair<iVec2, ResourceTile>& p) -> int
			{
				return acc + static_cast<int>((p.second.type == Type) * p.second.amount);
					
			});
	}

	[[nodiscard]] int getAllRessource() const {
		return
			getResourceCount(ResourceTile::ResType::WOOD) +
			getResourceCount(ResourceTile::ResType::COAL) +
			getResourceCount(ResourceTile::ResType::URANIUM);
	}

	[[nodiscard]] int getRessourceTile(iVec2 tile) {
		return m_AccessibleResourceTiles.at(tile).amount;
	}

	bool isInBounds(const iVec2 tile) const  {
		return
			(0 <= tile.first  && tile.first < width) &&
			(0 <= tile.second && tile.second < height)
		;
	}
	[[nodiscard]] bool isTileRes(const iVec2&tile)		const noexcept 	{	return m_AccessibleResourceTiles.contains(tile);}
	[[nodiscard]] bool isTilALLeRes(const iVec2& tile)	const noexcept	{	return m_AllResourceTiles.contains(tile);	}
	[[nodiscard]] bool isTileCity(const iVec2& tile)	const noexcept {	return m_cityTiles.contains(tile);	}

	// Call this between turns
	void clear() {
		m_cityTiles.clear();
		m_enemyCities.clear();
		m_AccessibleResourceTiles.clear();
		m_AllResourceTiles.clear();
		m_roads.clear();
	}

	void clearClusters() {	m_clusters.clear(); }
	void computeClusters(){	std::ranges::for_each(Clustering::generateAllExistingClusters(*this), [this](const Cluster& c) { m_clusters[c.getId()] = c; }); }
	void setClusters(const std::vector<Cluster>& clusters)	{	std::ranges::for_each(clusters, [this](const Cluster& c) { m_clusters[c.getId()] = c; });}

public:

	[[nodiscard]] bool isTileFree(iVec2 tile) const noexcept
	{
		return (!m_cityTiles.contains(tile) && !m_AccessibleResourceTiles.contains(tile));
	}

};
