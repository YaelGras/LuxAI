#pragma once


#include <lux/kit.hpp>
#include <lux/World/Map.h>
#include <string>
#include <tuple>
#include "lux/Blackboard/Blackboard.hpp"
#include "BehaviourTree.hpp"
#include "lux/World/Unit.h"
#include "lux/Utility/Cluster.h"
#include "lux/Thread/DataManager.h"
#include <set>
#include <unordered_map>
#include <algorithm>
#include "lux/kit/annotate.hpp"
#include <thread>
#include <ranges>
#include <chrono>
#include <mutex>
// Planner
#include "lux/Goap/FactoryAction.h"
#include "lux/Goap/Planner.h"
#include "lux/Goap/WorldState.h"
#include <lux/World/DayNightCycle.h>

#include "lux/CSU/CSU.h"

namespace classicalLeaf
{
	struct move_t {};
	struct buildCity_t {};
	struct research_t {};
	struct buildWorker_t {};
	struct cooldown_t {};
	struct fullResources_t {};
	struct notFullUnit_t {};
	struct notFullResearchPoint_t {};
	struct emptyTileUnderUnit_t {};
	struct onTargetTile_t {};
	struct pathFinder_t {};
	struct pathFollower_t {};
	struct pathAvailable_t {};
	struct getTarget_t {};
	struct majTarget_t {};
	struct haveTarget_t {};
	struct declareAlive_t {};
	struct planTarget_t {};
	struct isPlanning_t {};
}




class FactoryLeaf {
	DataManager& dataManager = DataManager::getInstance();;
	FactoryLeaf() = default;

	static std::mutex m_mutexMove;
public:

	FactoryLeaf& operator=(const FactoryLeaf&) = delete;
	FactoryLeaf(const FactoryLeaf&) = delete;

	static FactoryLeaf& getInstance() noexcept {
		static FactoryLeaf factory;
		return factory;
	}

	template<typename T>
	Leaf create() {
		return Leaf([](std::shared_ptr<BlackboardData>& data) -> bool { return false; });
	}

	
	/* Creation of leaf for having move order
	* Need in Blackboard :
	*	Unit : MovableUnit, Unit to move
	*	UnitPos : std::set<iVec2<int, int>>, pos of units
	*	DirectionMove : kit::DIRECTION, Direction to move
	*	NextTile : iVec2<int, int>, next tile to go
	*	Order : std::string, to return the order to the root of the behaviour tree
	*	
	*
	* Return : leaf when run return true if the transfer order can be create
	*	or false if information missing in blackboard
	*/
	template<>
	Leaf create<classicalLeaf::move_t>() {
		std::string name("Move");
		return Leaf([&](std::shared_ptr<BlackboardData>& data) -> bool {
			try {
				MovableUnit& unit = data->getData<MovableUnit>("Unit");
				std::multiset<iVec2>& unitpos = data->getData<std::multiset<iVec2>>("UnitsPos");
				DIRECTIONS& dir = data->getData<DIRECTIONS>("DirectionMove");
				std::string& order = data->getData<std::string>("Order");
				iVec2& NextTile = data->getData<iVec2>("NextTile");		

				std::lock_guard lock(m_mutexMove);
				if (auto it = unitpos.find(unit.getPos()); it != unitpos.end()) {					
					unitpos.erase(it);
				}
				order = kit::move(unit.getId(), dir);
				unitpos.insert(NextTile);			

				return true;
			}
			catch (TypeMismatch&) { return false; }
			catch (KeyNotFound&) { return false; }
			}, name );
	}


	/* Creation of leaf for building city order
	* Need in Blackboard :
	*	Unit : Unit, Unit who build the city
	*	Order : std::string, to return the order to the root of the behaviour tree
	*	NewCity : std::set<iVec2>, to insert the newCity for next turn
	*
	* Return : leaf when run return true if the build order can be create
	*	or false if information missing in blackboard
	*/
	template<>
	Leaf create<classicalLeaf::buildCity_t>() {
		std::string name("BuildCity");
		return Leaf([&](std::shared_ptr<BlackboardData>& data) -> bool {
			try {
				MovableUnit& unit = data->getData<MovableUnit>("Unit");
				auto& newCity = data->getData<std::set<iVec2>>("NewCity");
				// Modification of the order
				std::string& order = data->getData<std::string>("Order");
				dataManager.doneTarget(unit.getId());
				order = kit::buildCity(unit.getId());
				newCity.insert(unit.getPos());

				return true;
			}
			catch (const TypeMismatch&) { return false; }
			catch (const KeyNotFound&) { return false; }
			catch (const EEmptyTarget&) { return false; }
			}, name);
	}


	/* Creation of leaf for research on CityTile
	* Need in Blackboard :
	*	CityTile : CityTile, Tile to research this turn
	*	Order : std::string, to return the order to the root of the behaviour tree
	*	ResearchPoint : int, to update number of research point
	*
	* Return : leaf when run return true if the research order can be create
	*	or false if information missing in blackboard
	*/
	template<>
	Leaf create<classicalLeaf::research_t>() {
		std::string name("Research");
		return Leaf([](std::shared_ptr<BlackboardData>& data) -> bool {
			try {
				const CityTile& citytile = data->getData<CityTile>("CityTile");
				int& researchPoint = data->getData<int>("ResearchPoint");
				researchPoint++;
				// Modification of the order
				std::string& order = data->getData<std::string>("Order");
				order = kit::research(citytile.tilePos.first,
					citytile.tilePos.second);

				return true;
			}
			catch (TypeMismatch&) { return false; }
			catch (KeyNotFound&) { return false; }
			}, name);
	}

	/* Creation of leaf for building worker on CityTile
	* Need in Blackboard :
	*	CityTile : CityTile, Tile to buildWorker this turn
	*	Order : std::string, to return the order to the root of the behaviour tree
	*	NbUnit : size_t, to update number of unit
	*	NewCity : std::set<iVec2>, to erase the newCity for next turn
	*
	* Return : leaf when run return true if the buildWorker order can be create
	*	or false if information missing in blackboard
	*/
	template<>
	Leaf create<classicalLeaf::buildWorker_t>() {
		std::string name("BuildWorker");
		return Leaf([](std::shared_ptr<BlackboardData>& data) -> bool {
			try {
				CityTile& citytile = data->getData<CityTile>("CityTile");
				size_t& nbUnit = data->getData<size_t>("NbUnit");

				auto& newCity = data->getData<std::set<iVec2>>("NewCity");
				// Modification of the order
				std::string& order = data->getData<std::string>("Order");
				order = kit::buildWorker(citytile.tilePos.first,
					citytile.tilePos.second);
				nbUnit++;
				newCity.erase(citytile.tilePos);
				return true;
			}
			catch (TypeMismatch&) { return false; }
			catch (KeyNotFound&) { return false; }
			}, name);
	}


	/* Creation of leaf for knowing if cooldown of city tile or unit is under 1
	* Need in Blackboard :
	*	CooldDown : float, Cooldown of the tile or the unit
	*
	* Return : leaf when run : return true if cityTile or unit have cooldown < 1 otherwise false
	*	and false if information missing in blackboard
	*/
	template<>
	Leaf create<classicalLeaf::cooldown_t>() {
		std::string name("Cooldown");
		return Leaf([](std::shared_ptr<BlackboardData>& data) -> bool {
			try {
				float& cooldown = data->getData<float>("Cooldown");
				return cooldown < 1 ? true : false;
			}
			catch (TypeMismatch&) { return false; }
			catch (KeyNotFound&) { return false; }
			}, name);
	}

	/* Creation of leaf for knowing if unit is full
	* Need in Blackboard :
	*	Unit : MovableUnit, unit to know full or not
	*
	* Return : leaf when run : return true if unit is full otherwise false
	*/
	template<>
	Leaf create<classicalLeaf::fullResources_t>() {

		std::string name("FullResourcesUnit");

		return Leaf([](std::shared_ptr<BlackboardData>& data) -> bool {
			try {

				MovableUnit& unit = data->getData<MovableUnit>("Unit");


				RessourceCounts ressource = unit.getResources();
				return std::get<0>(ressource)
					+ std::get<1>(ressource)
					+ std::get<2>(ressource) == 100 ? true : false;
			}
			catch (TypeMismatch&) { return false; }
			catch (KeyNotFound&) { return false; }
			}, name);
	}

	/* Creation of leaf for knowing if tile is free (no ressource et no city on tile)
	* Need in Blackboard :
	*	Unit : Unit, unit where we want to know if tile is free
	*	GameMap: Map, Map of the game
	*
	* Return : leaf when run : return true if tile is free otherwise false
	*/
	template<>
	Leaf create<classicalLeaf::emptyTileUnderUnit_t>() {
		std::string name("Empty Tile Under Unit");
		return Leaf([](std::shared_ptr<BlackboardData>& data) -> bool {
			try {
				const MovableUnit& unit = data->getData<MovableUnit>("Unit");
				const Map& map = data->getData<Map>("GameMap");
				return map.isTileFree(unit.getPos());
			}
			catch (TypeMismatch&) { return false; }
			catch (KeyNotFound&) { return false; }
			}, name);
	}


	/* Creation of leaf for knowing if unit is on target tile
	* Need in Blackboard :
	*	Unit : Unit, unit where we want to know if he is on his target
	*
	* Return : leaf when run : return true if unit is on is target otherwise false
	*/
	template<>
	Leaf create<classicalLeaf::onTargetTile_t>() {
		std::string name("On Target Tile");
		return Leaf([&](std::shared_ptr<BlackboardData>& data) -> bool {
			try {
				const MovableUnit& unit = data->getData<MovableUnit>("Unit");
				const auto& target = dataManager.getNextTarget(unit.getId());

				return target.tile == unit.getPos();
			}
			catch (TypeMismatch&) { return false; }
			catch (KeyNotFound&) { return false; }
			catch (EEmptyTarget&) { return false; }
			}, name);
	}

	/* Creation of leaf for knowing if we have not max units
	* Need in Blackboard :
	*	GameMap : Map, Map of the game
	*	NewCity : std::set<iVec2>, to know if we have a new city
	*	NbUnit : size_t, to know number of unit
	*	CityTile : CityTile, to know if we are on a city tile
	*
	*
	* Return : leaf when run : return true if we haven't reached maximum
	*	units otherwise false
	*/
	template<>
	Leaf create<classicalLeaf::notFullUnit_t>() {
		std::string name("Not Full Unit");
		return Leaf([](std::shared_ptr<BlackboardData>& data) -> bool {
			try {
				Map& map = data->getData<Map>("GameMap");
				const auto& newCity = data->getData<std::set<iVec2>>("NewCity");
				const size_t& nbUnit = data->getData<size_t>("NbUnit");

				const CityTile& citytile = data->getData<CityTile>("CityTile");
				auto PlannedNbUnit = newCity.size() + nbUnit;

				if (nbUnit >= map.getAllCityTiles().size()) return false;

				if (newCity.contains(citytile.tilePos)) {
					return true;
				}



				//if(c.isNight() && c.timeBeforeDay() > 7) return false;
				return PlannedNbUnit < map.getAllCityTiles().size();
			}
			catch (TypeMismatch&) { return false; }
			catch (KeyNotFound&) { return false; }
			}, name);
	}

	
	/* Creation of leaf for knowing if we have max research point
	* Need in Blackboard :
	*	ResearchPoint : int, number of research point
	*
	* Return : leaf when run : return true if we haven't reached maximum
	*	research point otherwise false
	*/
	template<>
	Leaf create<classicalLeaf::notFullResearchPoint_t>() {
		std::string name ("Not Full Research");
		return Leaf([](std::shared_ptr<BlackboardData>& data) -> bool {
			try {
				int& researchPoint = data->getData<int>("ResearchPoint");

				return researchPoint < 200;
			}
			catch (TypeMismatch&) { return false; }
			catch (KeyNotFound&) { return false; }
			}, name);
	}

	
	/* Creation of leaf for find a path to a tile
* Need in Blackboard :
*	Unit : Unit, unit move
*	GameMap : Map, Map of the game
*	UnitsPos : std::multiset<iVec2<int, int>>, pos of units
*	StartTurnTime : std::chrono::high_resolution_clock::time_point, time of the start of the turn to block Astar
*
*
*
* Return : leaf when run : return true if success to add path
*/
	template<>
	Leaf create<classicalLeaf::pathFinder_t>() {
		std::string name("Path Finder");
		return Leaf([&](std::shared_ptr<BlackboardData>& data) -> bool {
			try {
				Map& map = data->getData<Map>("GameMap");
				MovableUnit& unit = data->getData<MovableUnit>("Unit");
				std::multiset<iVec2>& unitpos = data->getData<std::multiset<iVec2>>("UnitsPos");
				if (dataManager.emptyTargets(unit.getId())) return false;
				const iVec2& objectif = dataManager.getNextTarget(unit.getId()).tile;

				const auto& startTurn = data->getData<std::chrono::high_resolution_clock::time_point>("StartTurnTime");
				auto now = std::chrono::high_resolution_clock::now();
				auto maxTime = std::chrono::microseconds(48'000) - std::chrono::duration_cast<std::chrono::microseconds>(
					now - startTurn);

				if (maxTime < std::chrono::microseconds(0)) return false;
				//maxTime = std::min(std::chrono::microseconds(10'000), maxTime);
				dataManager.resetPath(unit.getId(), FactoryLeaf::AStar(unit.getPos(), objectif, map, 20, unitpos, maxTime));

				return true;
			}
			catch (TypeMismatch&) { return false; }
			catch (KeyNotFound&) { return false; }
			catch (EEmptyTarget&) { return false; }
			}, name);
	}
	
	/* Creation of leaf for following a path to a tile
* Need in Blackboard :
*	Unit : Unit, unit move
*	GameMap : Map, Map of the game
*	UnitsPos : std::multiset<iVec2<int, int>>, pos of units
*
* Insert :
*	NextTile : iVec2<int, int>, next tile to go
*	DirectionMove : DIRECTIONS, direction to move
*
* Return : leaf when run : return true if success to follow path
*/
	template<>
	Leaf create<classicalLeaf::pathFollower_t>() {
		std::string name("Path follower");
		return Leaf([&](std::shared_ptr<BlackboardData>& data) -> bool {
			try {
				std::multiset<iVec2>& unitpos = data->getData<std::multiset<iVec2>>("UnitsPos");
				MovableUnit unit = data->getData<MovableUnit>("Unit");
				Map& map = data->getData<Map>("GameMap");

				iVec2 nextTile = Coordinates::getNextTile(unit.getPos(), dataManager.tryNextMove(unit.getId()));


				{
					std::lock_guard lock(m_mutexMove);
					if (!map.isTileCity(nextTile) && unitpos.contains(nextTile)) return false; // OK 
				}
				if (map.getEnemyCityTiles().contains(nextTile)) return false;
				

				data->insertData("NextTile", nextTile);
				data->insertData("DirectionMove", dataManager.getNextMove(unit.getId()));
				return true;
			}
			catch (TypeMismatch&) { return false; }
			catch (KeyNotFound&) { return false; }
			catch (EEmptyPath&) { return false; }
			}, name);
	}
	
	/* Creation of leaf for knowing if exist a path to follow
* Need in Blackboard :
*	Unit : Unit, unit move
*
* Return : leaf when run : return true if success to follow path
*/
	template<>
	Leaf create<classicalLeaf::pathAvailable_t>() {
		std::string name("Path Available");
		return Leaf([&](std::shared_ptr<BlackboardData>& data) -> bool {
			try {
				MovableUnit& unit = data->getData<MovableUnit>("Unit");
				Map& map = data->getData<Map>("GameMap");
				return !dataManager.emptyPath(unit.getId());
			}
			catch (TypeMismatch&) { return false; }
			catch (KeyNotFound&) { return false; }
			}, name);
	}
	
	/* Creation of leaf for get an target for an unit
* Need in Blackboard :
*	Unit : Unit, unit move
*	GameMap : Map, Map of the game
*	UnitsPos : std::multiset<iVec2>, pos of units
*	Cluster : std::vector<Cluster>, clusters of the map
*	Turn : int, turn of the game
*	EnnemyPos : std::vector<iVec2>, pos of ennemy
*
* Return : leaf when run : return true if success to find a target
*/
	template<>
	Leaf create<classicalLeaf::getTarget_t>() {
		std::string name("Find Target");
		return Leaf([&](std::shared_ptr<BlackboardData>& data) -> bool {
			try {
				const MovableUnit& unit = data->getData<MovableUnit>("Unit");
				const Map& map = data->getData<Map>("GameMap");
				const std::multiset<iVec2>& unitpos = data->getData<std::multiset<iVec2>>("UnitsPos");
				const int& turn = data->getData<int>("Turn");
				const std::vector<Cluster>& clusters = data->getData<std::vector<Cluster>>("Cluster");
				const auto& enemyPos = data->getData<std::vector<iVec2>>("EnnemyPos");

				
				if (clusters.empty()) return false;

				dataManager.addToRun(unit.getId());

				std::vector<iVec2> allTargets;

				dataManager.getAllRefuelTargets(allTargets);

				if (turn > 5 && map.getAllCityTiles().contains(unit.getPos())) {
					const auto& city = map.getCityTile(unit.getPos());
					bool it = std::none_of(begin(allTargets), end(allTargets), [&](const iVec2& tile) {
						return city.m_city->contains(tile);
						});

					if (it && !city.m_city->hasEnoughFuel() && Coordinates::getLengthToTile(city.tilePos, Coordinates::getNearestTileOfType<ResourceTile>(map, city.tilePos)) < 3) {
						// REFUEL
						Target target{ TargetType::REFUEL, unit.getPos() };
						dataManager.addTarget(unit.getId(), target);
						return true;

					}
				}
				
				// BUILD


				allTargets.clear();
				dataManager.getAllBuildTargets(allTargets);
				iVec2 pos = unit.getPos();
				



				//Search of the nearest cluster of unit position
				int closestCluster = -1; 
				iVec2 closestBorder;
				int distance = 1000;

				for (int i = 0; i < clusters.size(); ++i) {
					auto nearestBorder = Coordinates::getClosestEmptyTileOfClusterBorder(pos, clusters[i], map, allTargets);
					if(!nearestBorder.has_value()) continue;
					int dist = Coordinates::getLengthToTile(nearestBorder.value(), pos);
					if (dist < distance) {
						distance = dist;
						closestBorder = nearestBorder.value();
						closestCluster = i;
					}
				}

				CSU& com = data->getData<CSU>("Commandant");

				constexpr static auto step = [](auto smaller, auto than) -> int {return (smaller <= than); };


				if (closestCluster == -1) { 
					return false; 
				}


				// Try to go in direction of enemies and stay around of the nearest cluster
				if(map.getAllCityTiles().size() < 3)
				{					
					if (enemyPos.size() > 0) {
						if(enemyPos[0].x != pos.x)
							pos.x += (enemyPos[0].x < pos.x) ? -1 : +1 ;
						if(enemyPos[0].y != pos.y)
							pos.y += (enemyPos[0].y < pos.y) ? -1 : +1 ;
					}
					auto c = Coordinates::getClosestEmptyTileOfClusterBorder(pos, clusters[closestCluster], map, allTargets);
					if(c.has_value()) closestBorder = c.value();

				}


				else if (closestBorder == pos)
				{
					return false;

				}


				if (com.isExplorer(unit)) {
					closestBorder = com.assignTileToExplorer(pos, clusters, map, allTargets);
				}

				
				dataManager.addTarget(unit.getId(), Target{ TargetType::BUILD, closestBorder });
				
				return true;
			}
			catch (TypeMismatch&) { std::cerr << "TypeMismatch" << std::endl;  return false; }
			catch (KeyNotFound&) { std::cerr << "KeyNotFound" << std::endl;  return false; }
			catch (EEmptyTarget&) { std::cerr << "EEmptyTarget" << std::endl; return false; }
			catch (EEmptyPath&) { std::cerr << "EEmptyPath" << std::endl; return false; }

			}, name);
	}

	/* Creation of leaf for maj target for unit
* Need in Blackboard :
*	Unit : MovableUnit, Unit to move
*	GameMap : Map, Map of the game
*	Cluster : std::vector<Cluster>, clusters of the map
*   Turn : int, turn of the game
*
*
* Return : True if he can Update target of the unit if it's necessary or false
*/
	template<>
	Leaf create<classicalLeaf::majTarget_t>() {
		std::string name("Maj Target");
		return Leaf([&](std::shared_ptr<BlackboardData>& data) -> bool {
			try {
				const MovableUnit& unit = data->getData<MovableUnit>("Unit");
				const Map& map = data->getData<Map>("GameMap");
				const std::vector<Cluster>& clusters = data->getData<std::vector<Cluster>>("Cluster");
				const int& turn = data->getData<int>("Turn");


				const auto& ennemy = map.getEnemyCityTiles();

				while (!dataManager.emptyTargets(unit.getId())) {
					const Target& target = dataManager.getNextTarget(unit.getId());
					switch (target.type) {
					case TargetType::BUILD:
						if (unit.getPos() == target.tile)
						{
							for (const auto& c : clusters)
							{
								iVec2 closest = Coordinates::getNearestBorderOfCluster(target.tile, c);
								if (auto dist = Coordinates::getLengthToTile(target.tile, closest); dist < 1)
								{
									return true; // maj success
								}							
							}	
							// We don't have resources next to us so we stop the target
							dataManager.doneTarget(unit.getId());
						}
						else if (map.isTileRes(target.tile) || ennemy.contains(target.tile)){
							// Impossible to build on the target
							dataManager.doneTarget(unit.getId());

						}
						else {
							// Valid target
							return true;
						}
						break;

					case TargetType::REFUEL:
						if (target.tile == unit.getPos())
						{
							if (map.isTileCity(target.tile))
							{
								auto city = map.getCityTile(target.tile).m_city;
								if (city/* && city->hasEnoughFuel()*/)
									// We have reached the target for refuel
									dataManager.doneTarget(unit.getId());
								return true;
							}
							else {
								// Impossible to refuel on the target where there is no city
								dataManager.doneTarget(unit.getId());
							}
						}
						else if(dataManager.existingTargetForOtherUnit(unit.getId(), target)) {
							dataManager.doneTarget(unit.getId());
						}
						else
							return true;
						break;

					case TargetType::MOVE:
						if (unit.getPos() == target.tile) {
							// We have reached the target
							dataManager.doneTarget(unit.getId());
							return true;
						}
						else if (ennemy.contains(target.tile)) {
							// Impossible to reach the target because of the ennemy's cities
							dataManager.doneTarget(unit.getId());
						}
						else {
							// Valid target
							return true;
						}
						break;						
					}
				} 
				return true;
				
			}
			catch (TypeMismatch&) { return false; }
			catch (KeyNotFound&) { return false; }
			catch (EEmptyTarget&) { return true; }
			}, name);
	}

	/* Creation of leaf for knowing if exist target for unit
	* Need in Blackboard :
	*	Unit : MovableUnit, Unit to know
	*
	*
	* Return : True if he there is taret for the unit false
	*/	
	template<>
	Leaf create<classicalLeaf::haveTarget_t>() {
		std::string name("Have Target");
		return Leaf([&](std::shared_ptr<BlackboardData>& data) -> bool {
			try {
				const MovableUnit& unit = data->getData<MovableUnit>("Unit");
				return !dataManager.emptyTargets(unit.getId());
				
			}
			catch (TypeMismatch&) { return false; }
			catch (KeyNotFound&) { return false; }
			catch (EEmptyTarget&) { return false; }
			}, name);
	}
	

	/* Creation of leaf for declare unit alive
	* Need in Blackboard :
	*	Unit : MovableUnit, Unit to know
	*
	*
	* Return : True if the target can run or false if the target is running in the background
	*/
	template<>
	Leaf create<classicalLeaf::declareAlive_t>() {
		std::string name("Declare alive");
		return Leaf([&](std::shared_ptr<BlackboardData>& data) -> bool {
			try {
				const MovableUnit& unit = data->getData<MovableUnit>("Unit");
				dataManager.setAlive(unit.getId());
				if(dataManager.isRunning(unit.getId())) return false;
				return true;
				
			}
			catch (TypeMismatch&) { return false; }
			catch (KeyNotFound&) { return false; }
			catch (EEmptyTarget&) { return false; }
			}, name);
	}
	
	/* Creation of leaf for plan target
	* Need in Blackboard :
	*	Unit : MovableUnit, Unit to know
	*	Cluster : std::vector<Cluster>, clusters of the map
	*	StartTurnTime : std::chrono::high_resolution_clock::time_point, time of the start of the turn to block GOAP
	*	GameMap : Map, Map of the game
	*
	*
	* Return : Return true if the plan can run and false otherwise
	*/
	template<>
	Leaf create<classicalLeaf::planTarget_t>() {
		std::string name("Plan Target");
		return Leaf([&](std::shared_ptr<BlackboardData>& data) -> bool {
			try {
				const MovableUnit& unit = data->getData<MovableUnit>("Unit");

				if(dataManager.emptyTargets(unit.getId())) return false;
				const Target&  target = dataManager.getNextTarget(unit.getId());
				const std::vector<Cluster>& clusters = data->getData<std::vector<Cluster>>("Cluster");
				if(Coordinates::getLengthToTile(unit.getPos(), target.tile) < 10 && target.type != TargetType::REFUEL)
					// No planner
					return true;

				const auto& startTurn = data->getData<std::chrono::high_resolution_clock::time_point>("StartTurnTime");
				const Map& map = data->getData<Map>("GameMap");


				auto plan =
					[target, map, unit, clusters, &startTurn] {

						Goap::WorldState ws;
						ws.setData("GameMap", map);
						ws.setData("Unit", unit);
						std::vector<iVec2> builded{};
						ws.setData("BuildedCity", builded);
						std::map<iVec2, float> refueled{};
						ws.setData("RefueledCity", refueled);


						std::vector<Goap::Action> actions;
						Goap::FactoryAction& factory = Goap::FactoryAction::getInstance();
						factory.createMoveToResources(unit.getPos(), target, clusters, actions);
						factory.createMoveToTile(target.tile, actions);



						Goap::WorldState goal; 
						std::vector<iVec2> buildedGoal; 
						std::map<iVec2, float> refueledGoal;
						if(target.type == TargetType::BUILD)
						{
							buildedGoal.emplace_back( target.tile );
							factory.createBuildCity(target.tile, actions);
							goal.setData("BuildedCity", buildedGoal);
						}
						else if (target.type == TargetType::REFUEL)
						{
							refueledGoal[target.tile] = std::min(map.getCityTile(target.tile).m_city->getFuelNeeded(), 400.f);
							factory.createRefuelCity(target.tile, actions);
							goal.setData("RefueledCity", refueledGoal);
						}
						
						MovableUnit copy = unit;
						copy.setPos(target.tile);
						copy.setResources({ 0,0,0 });
						goal.setData("Unit", copy);

						auto now = std::chrono::high_resolution_clock::now();
						auto maxTime = std::chrono::microseconds(48'000) - std::chrono::duration_cast<std::chrono::microseconds>(
							now - startTurn);

						if(maxTime < std::chrono::microseconds(0)) return;

						std::vector<Goap::Action> plan = Goap::Planner::plan(ws, goal, actions, maxTime);
						DataManager& dm = DataManager::getInstance();
						if(!plan.empty())
						{
							dm.resetTarget(unit.getId(), std::vector<Target>());
							
							for (const auto& action : plan)
								dm.addTarget(unit.getId(), action.getTarget());
														
							if (!(target == plan.back().getTarget()))
								dm.addTarget(unit.getId(), target);
						}
						else {
							std::cerr << "No plan" << std::endl;
						}
				};

				plan();
				return true;
				
			}
			catch (TypeMismatch&) { return false; }
			catch (KeyNotFound&) { return false; }
			catch (EEmptyTarget&) { return false; }
			}, name);
	}
	
	/* Creation of leaf for knowing if unit is currently planning
	* Need in Blackboard :
	*	Unit : MovableUnit, Unit to know
	*
	*
	* Return : True if the target is currently planning false otherwise
	*/
	template<>
	Leaf create<classicalLeaf::isPlanning_t>() {
		std::string name("Is Planning Target");
		return Leaf([&](std::shared_ptr<BlackboardData>& data) -> bool {
			try {
				const MovableUnit& unit = data->getData<MovableUnit>("Unit");
				

				return dataManager.isRunning(unit.getId());
				
			}
			catch (TypeMismatch&) { return false; }
			catch (KeyNotFound&) { return false; }
			catch (EEmptyTarget&) { return false; }
			}, name);
	}

	struct STileRecord
	{
		iVec2 tile;
		int estimatedTotalCost;
		int costSoFar;
		STileRecord* previousTile;

		bool operator<(const STileRecord& other) const
		{
			if(estimatedTotalCost != other.estimatedTotalCost)
				return (estimatedTotalCost < other.estimatedTotalCost);
			return tile < other.tile;
		}

		bool operator==(const STileRecord& other) const
		{
			return ((tile.x == other.tile.x) && (tile.y == other.tile.y));
		}
		bool operator==(const iVec2& other) const
		{
			return ((tile.x == other.x) && (tile.y == other.y));
		}
		bool operator!=(const iVec2& other) const
		{
			return !(*this == other);
		}
	};

	static std::vector<DIRECTIONS> AStar(
		const iVec2& start, 
		const iVec2& end, 
		const Map& map, 
		const int& maxTurn, 
		std::multiset<iVec2>& unitpos,
		const std::chrono::high_resolution_clock::duration& maxDuration
	);
};


