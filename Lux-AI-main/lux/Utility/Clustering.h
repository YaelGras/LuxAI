#pragma once

#include <vector>
#include "lux/Utility/Cluster.h"

class Map;

class Clustering
{
public:



	static std::vector<Cluster> generateClusters(const Map& map); 
	static std::vector<Cluster> generateAllExistingClusters(const Map& map);
	static int computeClusterValue(const Cluster& cluster);

	// Returns a direct border of the cluster. Does NOT account for cities, workers and stuff
	static std::vector<Tile> generateBordersOfCluster(const Cluster& cluster, const Map& map);

	/*		Generates a cluster based on a single resource tile, using flood fill algorithm.
	 *		It goes like this (X means that the tile has to be visited) :
	 *
	 *			. . O . O . .				. . X . O . .		. . X . O . .		. . X . O . .
	 *			. . O O O O . -> start		. . O O O O . ->	. . X O O O . ->	. . X X O O . -> ...
	 *			. . O . O . .				. . O . O . .		. . O . O . .		. . X . O . .
	 *
	 */
	static Cluster floodFillCluster(const ResourceTile& startTile, const Map& map);

	static Cluster floodFillClusterALLRessource(const ResourceTile& startTile, const Map& map);
};