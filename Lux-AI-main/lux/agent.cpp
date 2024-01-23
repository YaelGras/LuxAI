#include "lux/agent.hpp"
#include <tuple>
#include <set>
#include <fstream>
#include <thread>
#include <mutex>
#include "kit.hpp"
#include "lux/BehaviourTree/BehaviourTree.hpp"
#include "lux/Utility/Clustering.h"
#include "lux/Goap/Planner.h"
#include "lux/Goap/WorldState.h"
#include "lux/Goap/FactoryAction.h"
#include "lux/kit/annotate.hpp"
#define SEND_ORDER(s) orders.push_back(s);

#include <chrono>
using namespace std::chrono;

namespace kit
{
    void Agent::Initialize()
    {
        std::cerr << "Initialize" << std::endl;
        // get agent ID
        mID = std::stoi(kit::getline());
        std::string map_info = kit::getline();

        std::vector<std::string> map_parts = kit::tokenize(map_info, " ");

        int mapWidth = stoi(map_parts[0]);
        int mapHeight = stoi(map_parts[1]);
        m_gameMap.setSize({ mapWidth, mapHeight });
        m_influence.setSize({ mapWidth, mapHeight }); 
        
        m_behaviourTree = BehaviourTree::GetBehaviourTreeLuxAI();


        m_data->insertData("GameMap", m_gameMap);
        m_data->insertData("Units", m_movableUnits);        
        m_data->insertData("NbUnit", m_movableUnits.size());
        m_data->insertData("Turn", m_turn);
        m_data->insertData("ResearchPoint", m_researchPoints);
        m_data->insertData("Cluster", m_clusters);
        m_data->insertData("UnitsPos", m_unitpos);
        m_data->insertData("StartTurnTime", startTurn);
        m_data->insertData("NewCity", std::set<iVec2>{});


    }

    void Agent::ExtractGameState()
    {
            std::chrono::high_resolution_clock::time_point& newTurnTime =
                m_data->getData<std::chrono::high_resolution_clock::time_point>("StartTurnTime");

            newTurnTime = std::chrono::high_resolution_clock::now();
            startTurn = newTurnTime;
            Map& gameMap = m_data->getData<Map>("GameMap");
            std::unordered_map<std::string, MovableUnit>& movableUnits = 
                m_data->getData<std::unordered_map<std::string, MovableUnit>>("Units");

            auto& researchPoints = m_data->getData<int>("ResearchPoint");
            auto& nbunit = m_data->getData<size_t>("NbUnit");

            gameMap.clear();
            movableUnits.clear();
            m_cycle.incrementTurn();
            m_ennemyPos.clear();

            while (true)
            {
                std::string updateInfo = kit::getline();
                if (updateInfo == kit::INPUT_CONSTANTS::DONE)
                {
                    break;
                }
                std::vector<std::string> updates = kit::tokenize(updateInfo, " ");
                std::string input_identifier = updates[0];
                if (input_identifier == INPUT_CONSTANTS::RESEARCH_POINTS)
                {
                    int team = std::stoi(updates[1]);
                    int _researchPoints = std::stoi(updates[2]);
                    if (team == mID)
                    {
                        researchPoints = _researchPoints;
                        m_influence.setResearchPoint(researchPoints);
                    }
                }
                else if (input_identifier == INPUT_CONSTANTS::RESOURCES)
                {

                std::string type = updates[1];
                int x = std::stoi(updates[2]);
                int y = std::stoi(updates[3]);
                int amt = std::stoi(updates[4]);

                gameMap.updateALLTile(ResourceTile{ {x,y}, type, amt });

                    if (type == "coal" && researchPoints < 49) continue;
                    if (type == "uranium" && researchPoints < 200) continue;
                    gameMap.updateTile(ResourceTile{ {x,y}, type, amt });
                }
                else if (input_identifier == INPUT_CONSTANTS::UNITS)
                {
                    int i = 1;
                    int unittype = std::stoi(updates[i++]);
                    int team = std::stoi(updates[i++]);
                    std::string unitid = updates[i++];
                    int x = std::stoi(updates[i++]);
                    int y = std::stoi(updates[i++]);
                    float cooldown = std::stof(updates[i++]);
                    int wood = std::stoi(updates[i++]);
                    int coal = std::stoi(updates[i++]);
                    int uranium = std::stoi(updates[i++]);



                    if (team == mID)
                    {

                        movableUnits[unitid] = MovableUnit{
                            {x,y},
                            unitid,
                            cooldown
                        };

                        movableUnits[unitid].setResources({ wood,coal,uranium });
                        m_commandant.tellCommanderThatUnitIsAlive(m_movableUnits[unitid]);
                    }
                    else
                    {
                        m_ennemyPos.push_back({ x,y });
                    }



                }
                else if (input_identifier == INPUT_CONSTANTS::CITY)
                {

                    int i = 1;
                    int team = std::stoi(updates[i++]);
                    std::string cityid = updates[i++];
                    float fuel = std::stof(updates[i++]);
                    float lightUpkeep = std::stof(updates[i++]);
                    if (gameMap.getAllCities().contains(cityid)) {
                        gameMap.getCity(cityid).update(fuel, lightUpkeep);
                    }
                    else {
                        City c{ cityid, fuel, lightUpkeep };
                        gameMap.addCity(std::move(c));
                    }
                }
                else if (input_identifier == INPUT_CONSTANTS::CITY_TILES)
                {
                    int i = 1;
                    int team = std::stoi(updates[i++]);
                    std::string cityid = updates[i++];
                    int x = std::stoi(updates[i++]);
                    int y = std::stoi(updates[i++]);
                    float cooldown = std::stof(updates[i++]);

                    /* The city tile is from our team */
                    if (team == mID)
                    {
                        City& ct = gameMap.getCity(cityid);
                        iVec2 pos{ x,y };
                        CityTile c{ ct,pos,cooldown };
                        gameMap.updateTile(c);
                        ct.attachCityTile(gameMap.getCityTile(pos));

                    }

                    else
                    {
                        EnemyCityTile c{ {x,y} };
                        gameMap.updateEnnemyCityTile(c);
                    }

                }
                else if (input_identifier == INPUT_CONSTANTS::ROADS)
                {
                    int i = 1;
                    int x = std::stoi(updates[i++]);
                    int y = std::stoi(updates[i++]);
                    float road = std::stof(updates[i++]);

                    gameMap.updateRoad({ x,y }, road);

                }
            }

            nbunit = movableUnits.size();
               

    }


    //std::vector<Cluster> clustersALL;
    void Agent::GetTurnOrders(std::vector<std::string>& orders)
    {


        auto& clusters = m_data->getData<std::vector<Cluster>> ("Cluster");
        const auto& gameMap = m_data->getData<Map>("GameMap");
        m_gameMap.setClusters(clusters);
        clusters.clear();

        if (m_gameMap.getAccessibleResTiles().size()) m_gameMap.computeClusters();

        if (gameMap.getAccessibleResTiles().size())
            clusters = Clustering::generateClusters(gameMap);

        auto& turn = m_data->getData<int>("Turn");
        turn++;

        m_influence.updateAllInfluenceMap(clusters, m_ennemyPos, gameMap);

        std::ranges::for_each(m_movableUnits, [&](auto p) {
            m_commandant.assignSquadToUnit(m_movableUnits[p.first], p.second.getPos(), m_gameMap, turn);
            });

        m_gameMap.clearClusters();



       

        auto& unitpos = m_data->getData<std::multiset<iVec2>>("UnitsPos");
        unitpos.clear();

        const auto& units = m_data->getData<std::unordered_map<std::string, MovableUnit>>("Units");
        for (const auto& [id, unit] : units) {

            unitpos.insert(unit.getPos());
        }

        
        m_data->insertData("Orders", orders);
        m_data->insertData("EnnemyPos", m_ennemyPos);
        m_data->insertData("Cycle", m_cycle);
        m_data->insertData("Commandant", m_commandant);

        dm.resetAlive();
        m_behaviourTree.run(m_data);
        orders = m_data->getData<std::vector<std::string>>("Orders");


        if (duration_cast<milliseconds>(startTurn - std::chrono::high_resolution_clock::now()) > std::chrono::milliseconds(50)) {
            dm.cleanDead();
            dm.clearToRun();

            std::cerr << "Turn " << turn << " : " << duration_cast<milliseconds>(std::chrono::high_resolution_clock::now() - startTurn).count() << "ms" << std::endl;
            return;
		}

        // Planner part
        std::set<std::string> toRun = dm.getAllToRun();

        std::mutex orderMutex;
        std::vector<std::thread> threads;
        for (const auto& id : toRun) {
			threads.emplace_back([&](const std::string s) {

                DataManager::getInstance().startThread(s);

                auto m_plannerTree = BehaviourTree::GetBehaviourPlanner();
                const auto& units = m_data->getData<std::unordered_map<std::string, MovableUnit>>("Units");
                std::shared_ptr<BlackboardData> child_data = std::make_shared<BlackboardData>(m_data, false);
                child_data->insertData("Unit", units.at(s));
                std::string order;
                child_data->insertData("Order", order);

                bool success = m_plannerTree.run(child_data);

                if (success) {
                    std::lock_guard lock(orderMutex);
					orders.push_back(child_data->getData<std::string>("Order"));
				}

                DataManager::getInstance().endThread(s);
				
            }, id);

		}

        for (auto& t : threads) {
			t.join();
		}


        dm.clearToRun();
        dm.cleanDead();

       

#ifndef NDEBUG
            
            std::ofstream outFile("Targets.txt", std::ios::app);
            
            outFile << "Turn " << turn << std::endl;
            const auto& targetInfos = dm.getAllInfosTargets();
			for (const auto& targets : targetInfos)
			{
                outFile << targets.first << std::endl;
                for(int i = 0; i < targets.second.size(); i++)
                {
                    auto& t = targets.second[i];
                    if(t.type == TargetType::BUILD)
						orders.push_back(lux::Annotate::circle(t.tile.x, t.tile.y));
					else if (t.type == TargetType::REFUEL)
						orders.push_back(lux::Annotate::x(t.tile.x, t.tile.y));
            
                    orders.push_back(lux::Annotate::text(t.tile.x, t.tile.y, targets.first));
                    if (i > 0){
                       
                            orders.push_back(lux::Annotate::line(
                                targets.second[i - 1].tile.x, targets.second[i - 1].tile.y,
                                t.tile.x, t.tile.y));
                    }
                    outFile << "\t" << static_cast<char>(t.type) << " " << t.tile << std::endl;
                    
                }
            
			}

     /*   auto t = dm.getAllInfosTargets();
        
        
        std::ranges::for_each(m_gameMap.getClustersAsVector(), [&](const Cluster& c)
            {
                std::ranges::for_each(c.getResources(), [&](const ResourceTile& res)
                    {
        
                        orders.push_back(lux::Annotate::text(res.tilePos.x, res.tilePos.y, std::to_string(c.getId())));
                    });
            });
        
        std::ranges::for_each(m_commandant.getConquerors().units, [&](const id_t& unit) {
            for (auto& tt : t)
            {
                if (tt.first != unit.getId()) continue;
                if (tt.second.empty()) continue;
                orders.push_back(lux::Annotate::circle(unit.getPos().x, unit.getPos().y));
                orders.push_back(lux::Annotate::x(tt.second[0].tile.x, tt.second[0].tile.y));
            }
            });
        
        std::ranges::for_each(m_commandant.getBuilderSquads(), [&](const BuilderSquad& b) {
        
            std::ranges::for_each(b.units, [&](const Unit& unit)
                {
                    orders.push_back(lux::Annotate::text(unit.getPos().x, unit.getPos().y, std::to_string(b.assignedCluster)));
                });
            });*/
#endif      
            std::cerr << "Turn " << turn << " : " << duration_cast<milliseconds>(std::chrono::high_resolution_clock::now() - startTurn).count() << "ms" << std::endl;
    }

   
}

