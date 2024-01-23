#pragma once
#include <map>
#include <set>
#include <string>
#include <unordered_set>
#include <variant>

#include "lux/Utility/Cluster.h"
#include "lux/World/Tiles.h"
#include "lux/World/Unit.h"


// Sadly i lost my other file
// i HATE THIS ENVIRONMENT ITS SO BAD

//////////////////////////////////////////////////////////////////////////////////////////////////////
/// -- Squad definitions

using id_t = Unit;
using cluster_id_t = int;
enum SquadType
{
	BUILDER,FUELER,EXPLORER
};


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

struct Squad
{
	SquadType type;
	std::unordered_set<id_t> units;

};

struct FuelerSquad : Squad
{
	cluster_id_t assignedCluster;
};

struct ConquerorSquad : Squad
{
	cluster_id_t destCluster;
	std::set<int> clustersDestinations;
};

struct BuilderSquad : Squad				
{
	cluster_id_t assignedCluster;
	BuilderSquad(const cluster_id_t cluster) { assignedCluster = cluster;	type = BUILDER; }
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

//////////////////////////////////////////////////////////////////////////////////////////////////////
/// -- Commandant class

class Map;

class CSU
{
private:



	ConquerorSquad					m_conquerors;
	std::vector<BuilderSquad>		m_builders;
	std::vector<FuelerSquad>		m_fuelers;

	using keySquad = std::pair<SquadType, int>;
	std::map<std::string, keySquad> m_allUnitsEverAssigned ;



	std::size_t m_aliveUnitsCount;
	std::set<id_t> m_aliveUnits;

public:

	CSU() = default;

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// -- Getters

	[[nodiscard]] bool isExplorer(const id_t& id)						const noexcept	{	return m_conquerors.units.contains(id); }
	[[nodiscard]] bool unitIsAlreadyAssigned(const id_t& unit)			const noexcept { return m_allUnitsEverAssigned.contains(unit.getId());	}
	[[nodiscard]] bool unitIsAlive(const id_t& id)						const noexcept { return m_aliveUnits.contains(id);	}
	[[nodiscard]] const ConquerorSquad& getConquerors()					const noexcept { return m_conquerors;  }
	[[nodiscard]] const std::vector<BuilderSquad>& getBuilderSquads()	const noexcept { return m_builders;  }
	[[nodiscard]] std::variant<ConquerorSquad, BuilderSquad, FuelerSquad> getSquadOfUnit(const id_t& id);

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// -- Methods


	void tellCommanderThatUnitIsAlive(const id_t& unit)	noexcept { m_aliveUnits.insert(unit); }
	SquadType assignSquadToUnit(const id_t& unit, const iVec2& spawnTile, const Map& map, int turn);
	void clear();
	bool shouldBeExplorer();

	// -- Give target

	iVec2 assignTileToExplorer(iVec2 pos, const std::vector<Cluster>& clusters, const Map& map, const std::vector<iVec2>& allTargets);
	ConquerorSquad& getConquerors() { return m_conquerors; }

};
