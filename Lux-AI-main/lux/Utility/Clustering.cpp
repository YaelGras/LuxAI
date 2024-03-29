#include "Clustering.h"

#include <vector>
#include <numeric>

#include "lux/Utility/Cluster.h"
#include "lux/World/Coordinates.h"
#include "lux/World/Map.h"


std::vector<Cluster> Clustering::generateClusters(const Map& map)
{
	std::vector<Cluster> res;
	std::map<iVec2, unsigned int> tilesTagMap;

	unsigned int clusterTag = 1;
	for (int y = 0; y < map.getSize().y; ++y)
	{
		for (int x = 0; x < map.getSize().x; ++x)
		{
			// Discard non-res tiles
			iVec2 currentTile = { x,y };
			if (!map.isTileRes(currentTile)) continue;
			if (tilesTagMap.contains(currentTile)) continue;

			Cluster c = Clustering::floodFillCluster(map.getResourceTile(currentTile), map);
			std::vector<Tile> borders = Clustering::generateBordersOfCluster(c, map);

			const int resourceValue = Clustering::computeClusterValue(c);
			c.setValue(resourceValue);
			c.setBorders(borders);
			c.setId(clusterTag);
			res.push_back(c);

			for (const auto& resTile : c.getResources())
			{
				tilesTagMap[resTile.tilePos] = clusterTag;
			}

			clusterTag++;

		}

	}


	return res;
}

std::vector<Cluster> Clustering::generateAllExistingClusters(const Map& map)
{
	std::vector<Cluster> res;
	std::map<iVec2, unsigned int> tilesTagMap;

	unsigned int clusterTag = 1;
	for (int y = 0; y < map.getSize().y; ++y)
	{
		for (int x = 0; x < map.getSize().x; ++x)
		{
			// Discard non-res tiles
			iVec2 currentTile = { x,y };
			if (!map.isTilALLeRes(currentTile)) continue;
			if (tilesTagMap.contains(currentTile)) continue;

			Cluster c = Clustering::floodFillClusterALLRessource(map.getALLResourceTile(currentTile), map);

			std::vector<Tile> borders = Clustering::generateBordersOfCluster(c, map);
			const int resourceValue = Clustering::computeClusterValue(c);
			c.setValue(resourceValue);
			c.setBorders(borders);
			c.setId(clusterTag);
			res.push_back(c);

			for (const auto& resTile : c.getResources())
			{
				tilesTagMap[resTile.tilePos] = clusterTag;
			}

			clusterTag++;

		};

	}

	return res;
}

int Clustering::computeClusterValue(const Cluster& cluster)
{
	constexpr int resourceFactorsToFuelFactors[4] = { 1,10,40, 0 };
	std::vector<ResourceTile> clusterTiles = cluster.getResources();
	return std::accumulate(clusterTiles.begin(), clusterTiles.end(), 0, [&](int acc, auto resource) {
		return acc + resource.amount * resourceFactorsToFuelFactors[static_cast<int>(resource.type)];
		});

}

// Returns a direct border of the cluster. Does NOT account for cities, workers and stuff
std::vector<Tile> Clustering::generateBordersOfCluster(const Cluster& cluster, const Map& map)
{
	std::vector<Tile> res;
	const std::vector<ResourceTile>& clusterTiles = cluster.getResources();

	std::set<iVec2> borders;

	for (const auto& tile : clusterTiles)
	{
		for (
			const auto& neighbors = Coordinates::getNeighborsTile(tile.tilePos);
			const iVec2 & n : neighbors)
		{
			if (map.isInBounds(n) && !map.isTileRes(n))
				borders.insert(n);
		}
	}

	res.resize(borders.size());
	std::ranges::transform(borders, res.begin(), [&](const iVec2& t) {return Tile{ t }; });
	return res;
}



/*		Generates a cluster based on a single resource tile, using flood fill algorithm.
 *		It goes like this (X means that the tile has to be visited) :
 *
 *			. . O . O . .				. . X . O . .		. . X . O . .		. . X . O . .
 *			. . O O O O . -> start		. . O O O O . ->	. . X O O O . ->	. . X X O O . -> ...
 *			. . O . O . .				. . O . O . .		. . O . O . .		. . X . O . .
 *
 */
Cluster Clustering::floodFillCluster(const ResourceTile& startTile, const Map& map)
{
	Cluster res;


	std::set<iVec2> openTiles;
	openTiles.insert(startTile.tilePos);


	for (const iVec2 currentTile : openTiles)
	{
		const auto& neighbors = std::move(Coordinates::getNeighborsTile(currentTile));

		for (const iVec2& n : neighbors)
		{
			if (map.isTileRes(n))
			{
				openTiles.insert(n);
			}
		}

	}

	std::ranges::for_each(openTiles, [&res, &map](const iVec2 t) {res.addResourceTile(map.getResourceTile(t)); });

	return res;
}

Cluster Clustering::floodFillClusterALLRessource(const ResourceTile& startTile, const Map& map)
{
	Cluster res;


	std::set<iVec2> openTiles;
	openTiles.insert(startTile.tilePos);

	for (const iVec2 currentTile : openTiles)
	{
		const auto& neighbors = std::move(Coordinates::getNeighborsTile(currentTile));

		for (const iVec2& n : neighbors)
		{
			if (map.isTilALLeRes(n))
			{
				openTiles.insert(n);
			}
		}

	}

	std::ranges::for_each(openTiles, [&res, &map](const iVec2 t) {res.addResourceTile(map.getALLResourceTile(t)); });

	return res;
}

