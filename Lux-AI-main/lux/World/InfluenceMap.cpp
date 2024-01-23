#include "InfluenceMap.h"

void InfluenceMap::updateRessources(const Map& map)
{
	nbWoodTot = map.getResourceCount(ResourceTile::WOOD);
	nbCoalTot = map.getResourceCount(ResourceTile::COAL);
	nbUraniumTot = map.getResourceCount(ResourceTile::URANIUM);
}

void InfluenceMap::updateInfluenceMapRessources(std::vector<Cluster> cluster, const Map& map)
{
	m_clusters.clear();
	m_clustersUtility.clear();
	m_clusters = cluster;


	std::for_each(m_clusters.begin(), m_clusters.end(), [&](Cluster& cluster) {
		float utility = static_cast<float>(cluster.getValue());

		auto resType = cluster.getResourceType();
		Cluster::cluster_id_t id = cluster.getId();


		if (resType == ResourceTile::WOOD) {
			utility /= static_cast<float>(nbWoodTot);
		}
		else if (resType == ResourceTile::COAL) {
			utility /= 10.0f;
			utility /= static_cast<float>(nbCoalTot);
			float utilityRes{};
			if (m_researchPoint > 50)
				utilityRes = 1.0f;
			else
				utilityRes = static_cast<float>(m_researchPoint) / 100.0f;
			utility = (utility + utilityRes) / 2.0f;
		}
		else if (resType == ResourceTile::URANIUM) {
			utility /= 40.0f;
			utility /= static_cast<float>(nbUraniumTot);
			float utilityRes{};
			if (m_researchPoint > 200)
				utilityRes = 1.0f;
			else
				utilityRes = static_cast<float>(m_researchPoint) / 200.0f;
			utility = (utility + utilityRes) / 2.0f;
		}


		m_clustersUtility[id] = utility;
		});
}

void InfluenceMap::updateInfluenceMapEnemy(std::vector<iVec2> ennemyPos, const Map& map)
{
	
	m_influenceMapEnemy.clear();
	
	std::unordered_map<iVec2, EnemyCityTile> enemyCityTiles = map.getEnemyCityTiles();

	std::for_each(ennemyPos.begin(), ennemyPos.end(), [&](iVec2& pos) {
		m_influenceMapEnemy[pos] = 1.0f;
		});

	std::for_each(enemyCityTiles.begin(), enemyCityTiles.end(), [&](std::pair<iVec2, EnemyCityTile> city) {
		if (m_influenceMapEnemy.find(city.first) == m_influenceMapEnemy.end())
			m_influenceMapEnemy[city.first] += 1.0f;
		else
			m_influenceMapEnemy[city.first] = 1.0f;
		});

	std::vector<iVec2> direction = { {-1,0}, {1,0}, {0,-1}, {0,1} };
	std::unordered_map <iVec2, float> tempMap = m_influenceMapEnemy;

	for (int i = 0; i < m_mapSize.first; i++) {
		for (int j = 0; j < m_mapSize.second; j++) {
			iVec2 pos = { i,j };
			//Ajouter toutes les tile de la map dans l'influence map
			float influencetemp = m_influenceMapEnemy[pos];
			for (const auto& dir : direction) {
				iVec2 newPos = pos + dir;
				if (map.isInBounds(newPos)) {
					//if (m_influenceMapEnemy.find(newPos) != m_influenceMapEnemy.end()) {
					if (m_influenceMapEnemy.count(newPos) > 0) {
						influencetemp += m_influenceMapEnemy[newPos];
					}
				}
			}
			tempMap[pos] = influencetemp;
		}
	}

	m_influenceMapEnemy = tempMap;

	//Normalize m_influenceMapEnemy
	float max = 0.0f;
	std::for_each(m_influenceMapEnemy.begin(), m_influenceMapEnemy.end(), [&](std::pair<iVec2, float> tile) {
		if (tile.second > max)
			max = tile.second;
		});

	std::for_each(m_influenceMapEnemy.begin(), m_influenceMapEnemy.end(), [&](std::pair<iVec2, float> tile) {
		m_influenceMapEnemy[tile.first] = tile.second / max;
		});

	
}

void InfluenceMap::updateinfluenceResMinable(const Map& map)
{
	m_influenceResMinable.clear();

	std::unordered_map<iVec2, ResourceTile> resTiles = map.getAllResTiles();

	std::vector<iVec2> direction = { {-1,0}, {1,0}, {0,-1}, {0,1}, {0,0} };

	for (int i = 0; i < m_mapSize.first; i++) {
		for (int j = 0; j < m_mapSize.second; j++) {
			iVec2 pos = { i,j };
			int value = 0;
			for (const auto& dir : direction) {
				iVec2 newPos = pos + dir;
				if (map.isInBounds(newPos)) {
					//if (resTiles.find(newPos) != resTiles.end()) {
					if(resTiles.count(newPos) > 0) {
						if (resTiles[newPos].type == ResourceTile::WOOD) {
							if (resTiles[newPos].amount - 20 >= 0)
								value += 20;
							else
								value += std::abs(resTiles[newPos].amount - 20);
						}
						else if (resTiles[newPos].type == ResourceTile::COAL && m_researchPoint >= 50) {
							if (resTiles[newPos].amount - 5 >= 0)
								value += 5;
							else
								value += std::abs(resTiles[newPos].amount - 5);
						}
						else if (resTiles[newPos].type == ResourceTile::URANIUM && m_researchPoint >= 200) {
							if (resTiles[newPos].amount - 2 >= 0)
								value += 2;
							else
								value += std::abs(resTiles[newPos].amount - 2);
						}
					}
				}
			}
			m_influenceResMinable[pos] = value;
		}
	}
}

void InfluenceMap::updateinfluenceResFuel(const Map& map)
{
	m_influenceResFuel.clear();

	std::unordered_map<iVec2, ResourceTile> resTiles = map.getAllResTiles();

	for (int i = 0; i < m_mapSize.first; i++) {
		for (int j = 0; j < m_mapSize.second; j++) {
			iVec2 pos = { i,j };
			if (resTiles.find(pos) != resTiles.end()) {
				if (resTiles[pos].type == ResourceTile::COAL) {
					m_influenceResFuel[pos] = resTiles[pos].amount * 10;
				}
				else if (resTiles[pos].type == ResourceTile::URANIUM) {
					m_influenceResFuel[pos] = resTiles[pos].amount * 40;
				}
				else {
					m_influenceResFuel[pos] = resTiles[pos].amount;
				}
			}
			else {
				m_influenceResFuel[pos] = 0;
			}
		}
	}
}

void InfluenceMap::updateAllInfluenceMap(const std::vector<Cluster>& cluster, const std::vector<iVec2>& ennemyPos, const Map& map)
{
	updateRessources(map);
	updateInfluenceMapRessources(cluster, map);
	updateInfluenceMapEnemy(ennemyPos, map);
	updateinfluenceResMinable(map);
	updateinfluenceResFuel(map);
}
