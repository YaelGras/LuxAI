#include "FactoryLeaf.hpp"

std::mutex FactoryLeaf::m_mutexMove{};

std::vector<DIRECTIONS> FactoryLeaf::AStar(
	const iVec2& start,
	const iVec2& end,
	const Map& map,
	const int& maxTurn,
	std::multiset<iVec2>& unitpos, 
	const std::chrono::high_resolution_clock::duration& maxDuration
)
{
	auto begin = std::chrono::high_resolution_clock::now();
	STileRecord startRecord{ start, Coordinates::getLengthToTile(start, end), 0 };

	std::vector<STileRecord> open{};

	open.push_back(startRecord);


	std::vector<STileRecord> closed{};

	STileRecord currentTile;

	do {
		currentTile = *std::min_element(open.begin(), open.end());

		if (currentTile == end)
		{
			break;
		}
		std::array<iVec2, 4> neighbours = Coordinates::getNeighborsTile(currentTile.tile);

		int cost = currentTile.costSoFar + 1;

		if (cost <= maxTurn)
			for (size_t i = 0; i < neighbours.size(); i++)
			{
				iVec2 t = neighbours[i];
				if (!map.isInBounds(t) || map.getEnemyCityTiles().contains(t) /*|| unitpos.contains(t)*/)
					continue;

				int heuristicCost;

				// Tile already processed
				if (auto it = std::find(closed.begin(), closed.end(), STileRecord{ t });
					it != closed.end()) {
					if (it->costSoFar < cost)
						continue;
					heuristicCost = (it->estimatedTotalCost - it->costSoFar);
					closed.erase(it);
				}
				// Waiting tile to be processed 
				else if ((it = std::find(open.begin(), open.end(), STileRecord{ t })) != open.end()) {
					if (it->costSoFar < cost)
						continue;
					heuristicCost = (it->estimatedTotalCost - it->costSoFar);
					open.erase(it);
				}
				// First visit for the tile
				else {
					heuristicCost = Coordinates::getLengthToTile(t, end);
				}

				STileRecord NewRecord{ t,  cost + heuristicCost, cost, new STileRecord(currentTile) };
				open.push_back(NewRecord);
			}

		closed.push_back(currentTile);
		std::swap(*std::find(open.begin(), open.end(), currentTile), open.back());
		open.pop_back();

		auto now = std::chrono::high_resolution_clock::now();
		if (std::chrono::duration_cast<std::chrono::microseconds>(now - begin) >
			std::chrono::duration_cast<std::chrono::microseconds>(maxDuration)) {
			std::cerr << "Timeout\n";
			break;
		}

	} while (!open.empty());

	if (currentTile != end)
	{

		currentTile = *std::min_element(closed.begin(), closed.end());
	}

	// Get the path 
	std::vector<DIRECTIONS> path;
	while (currentTile != start) {
		path.push_back(Coordinates::getDirection(currentTile.previousTile->tile, currentTile.tile));
		currentTile = *currentTile.previousTile;
	}

	std::reverse(path.begin(), path.end());
	return path;
}