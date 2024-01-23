#pragma once

#include <map>
#include <vector>
#include <variant>
#include <array>
#include <set>
#include <queue>

#include "Map.h"
#include "Tiles.h"

#include "lux/Utility/Cluster.h"

class Coordinates {


public:

	// THIS DOES NOT CHECK IF THE neighbor EXISTS, JUST RETURNS AN ARRAY OF POSSIBLE TILES
	// N -> E -> S -> W
	static std::array<iVec2, 4> getNeighborsTile(const iVec2 tile)
	{
		return {

			getNextTile(tile, DIRECTIONS::NORTH),
			getNextTile(tile, DIRECTIONS::EAST),
			getNextTile(tile, DIRECTIONS::SOUTH),
			getNextTile(tile, DIRECTIONS::WEST)

		};
	}


	static DIRECTIONS getDirection(iVec2 tile, iVec2 neighbor) {

		if (neighbor.first - tile.first == 1) return DIRECTIONS::EAST;
		if (neighbor.first - tile.first == -1) return DIRECTIONS::WEST;
		if (neighbor.second - tile.second == -1) return DIRECTIONS::NORTH;
		if (neighbor.second - tile.second == 1) return DIRECTIONS::SOUTH;

		return DIRECTIONS::CENTER;

	}

	// This doesn't check if the tile is in bounds or not
	[[nodiscard]] static iVec2 getNextTile(const iVec2 tile, DIRECTIONS dir)
	{

		static std::map<char, iVec2> lookup = {
			{'n',{0,-1}},
			{'e',{1,0}},
			{'s',{0,1}},
			{'w',{-1,0}},
			{'c',{0,0}},
		};

		return tile + lookup[static_cast<char>(dir)];

	}

	[[nodiscard]] static iVec2 getStepsToTile(const iVec2 from, const iVec2 to) { return to - from; }

	static int getLengthToTile(const iVec2 from, const iVec2 to)
	{
		const iVec2 steps = getStepsToTile(from, to);
		return abs(steps.first) + abs(steps.second);
	}



	// Use this with caution
	// Call std::holds_alternative<tileType>(array[i]) before using it !
	// https://en.cppreference.com/w/cpp/utility/variant

	// NOTE : This has not been tested yet
	static std::array<varTile, 4> getCrossNeighbor(iVec2 tile, const Map& map)
	{

		auto getVarTile = [&map](iVec2 neigh) -> varTile
			{

				if (!map.isInBounds(neigh)) return OOBTile{};

				const auto& cities = map.getAllCityTiles();
				const auto& res = map.getAccessibleResTiles();

				if (cities.contains(neigh)) return cities.at(neigh);
				if (res.contains(neigh)) return res.at(neigh);
				return Tile{ neigh };
			};

		return
		{
			getVarTile(getNextTile(tile, DIRECTIONS::NORTH)),
			getVarTile(getNextTile(tile, DIRECTIONS::EAST)),
			getVarTile(getNextTile(tile, DIRECTIONS::SOUTH)),
			getVarTile(getNextTile(tile, DIRECTIONS::WEST))
		};

	}




	static iVec2 getNearestBorderOfCluster(iVec2 unitPos, const Cluster& cluster)
	{
		return std::ranges::min_element(cluster.getBorder(),
			[&](auto& a, auto& b) -> bool
			{ return getLengthToTile(a.tilePos, unitPos) < getLengthToTile(b.tilePos, unitPos); }
		)->tilePos;
	}

	static std::optional<iVec2> getClosestEmptyTileOfClusterBorder(const iVec2 unitPos, const Cluster& cluster, const Map& map, const std::vector<iVec2>& targeted)
	{

		auto tile = std::ranges::min_element(
			cluster.getBorder(),
			std::less{},
			[&](const Tile& border) -> float {
				return	static_cast<float>(Coordinates::getLengthToTile(border.tilePos, unitPos))
					+	static_cast<float>(map.isTileCity(border.tilePos) || map.getEnemyCityTiles().contains(border.tilePos)
						|| (border.tilePos != unitPos && std::find(targeted.begin(), targeted.end(), border.tilePos) != targeted.end()))*500.f;
			}
		)->tilePos;
		const bool isCity = map.isTileCity(tile);
		const bool isEnemyCity = map.getEnemyCityTiles().contains(tile);
		const bool isTargeted = std::ranges::find(targeted, tile) != targeted.end();
		if (isCity || isEnemyCity || isTargeted) {
			return std::nullopt;
		}

		return tile;
	}

	// used for explorers
	static std::optional<iVec2> getAccessibleEmptyTileOfFarBorder(const iVec2 unitPos, const Cluster& cluster, const Map& map, int maxDist)
	{
		constexpr static auto step = [](auto smaller, auto than) -> int {return (smaller <= than); };

		auto tile = std::ranges::max_element(
			cluster.getBorder(),
			std::less{},
			[&](const Tile& border) -> int {

				const int dist = Coordinates::getLengthToTile(border.tilePos, unitPos);
				const int available = (map.isTileCity(border.tilePos) || map.getEnemyCityTiles().contains(border.tilePos));
				return	(dist + available) * step(dist, maxDist);
			}
		)->tilePos;
		const bool isCity = map.isTileCity(tile);
		const bool isEnemyCity = map.getEnemyCityTiles().contains(tile);
		if (isCity || isEnemyCity) {
			return std::nullopt;
		}

		return tile;
	}





private:
	template<class TileType>
	static iVec2 getNearestTileOfType(const Map& map, const iVec2 from);

public:

	template<>
	static iVec2 getNearestTileOfType<CityTile>(const Map& map, const iVec2 from) {

		return std::ranges::min_element(map.getAllCityTiles(), [&](auto& a, auto& b) -> bool
			{ return getLengthToTile(a.second.tilePos, from) < getLengthToTile(b.second.tilePos, from); }
		)->second.tilePos;
	}


	template<>
	static iVec2 getNearestTileOfType<ResourceTile>(const Map& map, const iVec2 from) {

		return std::ranges::min_element(map.getAccessibleResTiles(), [&](auto& a, auto& b) -> bool
			{ return getLengthToTile(a.second.tilePos, from) < getLengthToTile(b.second.tilePos, from); }
		)->second.tilePos;
	}

	template<>
	static iVec2 getNearestTileOfType<EmptyTile>(const Map& map, const iVec2 from) {

		const auto& resTiles = map.getAccessibleResTiles();
		const auto& cityTiles = map.getAllCityTiles();

		std::queue<iVec2> tilesToCheck;
		std::set<iVec2> checkedTiles;
		iVec2 currentTile{};

		tilesToCheck.push(from);

		while (!tilesToCheck.empty()) {


			currentTile = tilesToCheck.front();
			tilesToCheck.pop();
			checkedTiles.insert(currentTile);


			if (!resTiles.contains(currentTile) && !cityTiles.contains(currentTile)) break;


			for (const iVec2 neighbor : Coordinates::getNeighborsTile(currentTile)) {

				if (checkedTiles.contains(neighbor))	continue;
				if (!map.isInBounds(neighbor))			continue;
				tilesToCheck.push(neighbor);
			}

		}

		return currentTile;
	}

};
