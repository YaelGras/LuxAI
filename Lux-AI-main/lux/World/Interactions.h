#pragma once 
#include <optional>

#include <map>
#include "Tiles.h"
#include "Unit.h"
#include "Coordinates.h"


class Interactions {

public:

	// Returns null if no other unit is close to the unit, or a movable unit if there is one (going N->E->S->W)
	static std::optional<MovableUnit> getCloseUnit(const MovableUnit& unit
	                                               , const std::map<iVec2, MovableUnit>& others
	                                               , const Map& map)
	{

		for (iVec2 n : Coordinates::getNeighboursTile(unit.getPos()))
		{
			if (!map.isInBounds(n)) continue;
			if (others.contains(n)) return others.at(n);
		}

		return std::nullopt;

	}

};
