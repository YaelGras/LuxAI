#pragma once

#include <vector>
#include "lux/World/Tiles.h"

class Cluster
{
public:

	using cluster_id_t = int;

private:

	std::vector<ResourceTile> m_clusterTiles;
	std::vector<Tile> m_borderTiles;

	int m_currentResourceValue{};
	cluster_id_t m_id = -1;

public:

	Cluster() = default;
	Cluster(
		const std::vector<ResourceTile>& resTiles, 
		const std::vector<Tile>& borderTiles,
		const int value = 0
	)
		: m_clusterTiles(resTiles)
		, m_borderTiles(borderTiles)
		, m_currentResourceValue(value)
	{}

	////////////////////////////////////////////////////////////////////////////////////

	[[nodiscard]]	const std::vector<ResourceTile>&	getResources()		const noexcept { return m_clusterTiles; }
	[[nodiscard]]	const std::vector<Tile>&			getBorder()			const noexcept { return m_borderTiles; }
	[[nodiscard]]	int									getValue()			const noexcept { return m_currentResourceValue; }
	[[nodiscard]]	cluster_id_t						getId()				const noexcept { return m_id; }
	[[nodiscard]]	ResourceTile::ResType				getResourceType()	const noexcept { return m_clusterTiles[0].type; }
	[[nodiscard]]	bool								isEmpty()			const noexcept { return m_clusterTiles.empty(); }

	void addResourceTile(const ResourceTile& tile)		{ m_clusterTiles.push_back(tile); }
	void addBorderTile(const Tile& tile)				{ m_borderTiles.push_back(tile); }
	void setBorders(const std::vector<Tile>& borders)	{ m_borderTiles= borders; }
	void setValue(const int value)						{ m_currentResourceValue = value; }
	void setId(const int value)							{ m_id = value; }

	////////////////////////////////////////////////////////////////////////////////////

};