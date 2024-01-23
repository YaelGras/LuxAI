#ifndef agent_h
#define agent_h

#include <vector>
#include <unordered_map>
#include <set>
#include <memory>
#include <functional>

#include "CSU/CSU.h"
#include "lux/World/Unit.h"
#include "lux/World/Map.h"
#include "lux/World/DayNightCycle.h"
#include "lux/World/Influencemap.h"
#include "lux/BehaviourTree/BehaviourTree.hpp"
#include "lux/Thread/DataManager.h"
class Cluster;
#include "lux/Blackboard/Blackboard.hpp"

namespace kit
{
    class Agent
    {
    protected:
        int mID = 0;

        std::unordered_map<std::string, MovableUnit> m_movableUnits;

        Map m_gameMap{};
        DayNightCycle m_cycle;
        int m_researchPoints=0;

        InfluenceMap& m_influence = InfluenceMap::getInstance();

        std::vector<iVec2> m_ennemyPos{};

        BehaviourTree m_behaviourTree;
        BehaviourTree m_plannerTree;
        int m_turn = 0;
        std::vector<Cluster> m_clusters;

        std::shared_ptr<BlackboardData> m_data = std::make_shared<BlackboardData>(false);

        std::multiset<iVec2> m_unitpos;


        DataManager& dm = DataManager::getInstance();

        std::function<void(std::string&)> m_RunForPlan;


        std::chrono::high_resolution_clock::time_point startTurn;


        CSU m_commandant;

    public:

        // Returns true if there is less workers than cities, else false
        [[nodiscard]] bool canBuildWorker() const noexcept {
            return m_movableUnits.size() < m_gameMap
                .getAllCityTiles().size();
        }

    public:



        void Initialize();
        void ExtractGameState();
        void GetTurnOrders(std::vector<std::string>& orders);
    };
}
#endif
