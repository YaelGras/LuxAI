#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <algorithm>

#include "Tiles.h"
#include "Map.h"
#include "lux/Utility/Clustering.h"
#include "lux/Utility/Cluster.h"

class InfluenceMap
{
private:
	int nbWoodTot{}, nbCoalTot{}, nbUraniumTot{}, nbAllRessources{}, m_researchPoint{};

	std::vector<Cluster> m_clusters;
	std::unordered_map < Cluster::cluster_id_t, float > m_clustersUtility;

	std::unordered_map <iVec2, float> m_influenceMapEnemy;
	std::unordered_map <iVec2, int> m_influenceResMinable;
	std::unordered_map <iVec2, int> m_influenceResFuel;

	iVec2 m_mapSize{};

	InfluenceMap() = default;

public:

	static InfluenceMap& getInstance() {
		static InfluenceMap instance;
		return instance;
	}

	[[nodiscard]] const std::unordered_map < Cluster::cluster_id_t, float > getClustersUtility() const { return m_clustersUtility; }
	[[nodiscard]] const Cluster::cluster_id_t& getClusterId() const { return m_clustersUtility.begin()->first; }
	[[nodiscard]] const float& getClusterUtility() const { return m_clustersUtility.begin()->second; }
	[[nodiscard]] const std::vector<Cluster>& getClusters() const { return m_clusters; }
	[[nodiscard]] const ResourceTile::ResType& getClusterType() const { return m_clusters.begin()->getResourceType(); }

	[[nodiscard]] const std::unordered_map <iVec2, float>& getInfluenceMapEnemy() const{ return m_influenceMapEnemy; }

	[[nodiscard]] const std::unordered_map <iVec2, int>& getInfluenceResMinable() const { return m_influenceResMinable; }

	[[nodiscard]] const std::unordered_map <iVec2, int>& getInfluenceResFuel() const { return m_influenceResFuel; }


	[[nodiscard]] iVec2 getMapSize() { return m_mapSize; }

	void setResearchPoint(const int& researchPoint) { m_researchPoint = researchPoint; }
	void setSize(iVec2 size) { m_mapSize = size; }

	void updateRessources(const Map& map);

	void updateInfluenceMapRessources(std::vector<Cluster> cluster, const Map& map);

	void updateInfluenceMapEnemy(std::vector<iVec2> ennemyPos, const Map& map);

	void updateinfluenceResMinable(const Map& map);

	void updateinfluenceResFuel(const Map& map);

	void updateAllInfluenceMap(const std::vector<Cluster>& cluster, const std::vector<iVec2>& ennemyPos, const Map& map);

	void DisplayTypeCluster() {
		for (auto& cluster : m_clusters) {
			std::cerr << "Cluster " << cluster.getId() << " : " << cluster.getResourceType() << std::endl;
		}
	}

	void DisplayUtilityCluster() {
		for (auto& cluster : m_clustersUtility) {
			auto temp = std::find_if(m_clusters.begin(), m_clusters.end(), [&cluster](const Cluster& clusters) {
				return clusters.getId() == cluster.first;
				});
			std::cerr << "Cluster " << cluster.first << ", Type " << temp->getResourceType() << ": " << cluster.second << std::endl;
		}
	}
};
