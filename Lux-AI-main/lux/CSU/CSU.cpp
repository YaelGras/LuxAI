#include "CSU.h"

#include "lux/World/Coordinates.h"
#include "lux/World/Map.h"
#include "lux/World/DayNightCycle.h"



std::variant<ConquerorSquad, BuilderSquad, FuelerSquad> CSU::getSquadOfUnit(const id_t& id)
{
	std::variant <ConquerorSquad, BuilderSquad, FuelerSquad> res;

	if (m_conquerors.units.contains(id)) return res.emplace<ConquerorSquad>(m_conquerors);
	if (std::ranges::any_of(m_builders, [&](const BuilderSquad& b) {if (b.units.contains(id)) return (res.emplace<BuilderSquad>(b), true);	return false; }))
		if (std::ranges::any_of(m_fuelers, [&](const FuelerSquad& f) {if (f.units.contains(id)) return (res.emplace<FuelerSquad>(f), true);		return false; }))

			return res;
	return {};
}


SquadType CSU::assignSquadToUnit(const id_t& unit, const iVec2& spawnTile, const Map& map, int turn)
{
	if (map.getClusters().empty())
	{
		return BUILDER;

	}
	//	std::cerr << "hello" << std::endl;
	if (m_builders.empty())
	{
		for (int i = 0; i < map.getClusters().size()+10; i++)
			m_builders.emplace_back(BuilderSquad{ i });

	}
	if (unitIsAlive(unit) && unitIsAlreadyAssigned(unit))
	{
		if (m_allUnitsEverAssigned[unit.getId()].first == EXPLORER)		{	m_conquerors.units.insert(unit);		}
		if (m_allUnitsEverAssigned[unit.getId()].first == BUILDER) 		{	auto& [type, index] = m_allUnitsEverAssigned[unit.getId()]; 	m_builders[index].units.insert(unit);	}
		return m_allUnitsEverAssigned[unit.getId()].first; // Not sure about this yet
	}

	DayNightCycle dnc{ turn };
	if (m_aliveUnits.size() > 10 && dnc.isDay())
	{
		m_conquerors.units.insert(unit);
		m_aliveUnitsCount++;
		m_allUnitsEverAssigned[unit.getId()] = { EXPLORER, 0 };
		return EXPLORER;
	}

	auto v = map.getClustersAsVector();
	
	cluster_id_t closestClusterId = std::ranges::min_element(
		v,
		[&](const Cluster& a, const Cluster& b)
		{
			return
				Coordinates::getLengthToTile(Coordinates::getNearestBorderOfCluster(unit.getPos(), a), unit.getPos()) <
				Coordinates::getLengthToTile(Coordinates::getNearestBorderOfCluster(unit.getPos(), b), unit.getPos());
		}
	)->getId();
	
	m_builders[closestClusterId].units.insert(unit);
	m_allUnitsEverAssigned[unit.getId()] = { BUILDER, closestClusterId };
	m_aliveUnitsCount++;

	return BUILDER;
}

void CSU::clear()
{
	m_aliveUnits.clear();
	std::ranges::for_each(m_builders, [&](BuilderSquad& b) {b.units.clear(); });
	m_conquerors.units.clear();
	m_conquerors.clustersDestinations.clear();

}

bool CSU::shouldBeExplorer() { return false; }

iVec2 CSU::assignTileToExplorer(iVec2 pos, const std::vector<Cluster>& clusters, const Map& map, const std::vector<iVec2>& allTargets)
{

	const Cluster& bestCluster = *std::ranges::max_element(
		clusters,
		std::greater{},
		[&](const Cluster& c) -> float
		{
			auto b = Coordinates::getClosestEmptyTileOfClusterBorder(pos, c, map, allTargets);
			if (!b.has_value()) return 0;
			int dist = Coordinates::getLengthToTile(b.value(), pos);
			return static_cast<float>(Coordinates::getLengthToTile(pos, b.value()) - (!m_conquerors.clustersDestinations.contains(c.getId()) * 3.f));
		});

	const iVec2 furthestAccessibleBorder = Coordinates::getClosestEmptyTileOfClusterBorder(pos, bestCluster, map, allTargets).value();
	m_conquerors.clustersDestinations.insert(bestCluster.getId());
	return furthestAccessibleBorder;
}
