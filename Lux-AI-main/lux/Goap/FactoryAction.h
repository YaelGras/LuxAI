#include "Action.h"
#include "WorldState.h"
#include "lux/kit.hpp"
#include "lux/World/Coordinates.h"
#include "lux/World/Map.h"
#include "lux/Utility/Cluster.h"
#include <vector>
#include <memory>
namespace Goap {

	namespace classicalCondition {
		// For unit
		struct HaveResources {};
		struct FullResources {};
		struct NotFullResources {};
		struct OnTile {};
		struct EmptyTile {};
		struct OnCity {};
	}

	namespace classicalEffects {
// For unit
		struct MoveOnTile{};
		struct collectResources{};
		struct Refuel{};
		struct BuildCity{};
	}



class FactoryCondition {
	FactoryCondition() = default;
	static FactoryCondition factory;
	
public:
	FactoryCondition& operator=(const FactoryCondition&) = delete;
	FactoryCondition(const FactoryCondition&) = delete;

	static FactoryCondition& getInstance() noexcept {
		return factory;
	}

	template<typename T>
	Condition createCondition() {
		return Condition("" ,
			[](const std::shared_ptr<WorldState>& worldState) {
				return false;
			}
		);
	}
	template<>
	Condition createCondition<classicalCondition::HaveResources>() {
		return Condition("HaveResources",
			[](const std::shared_ptr<WorldState>& worldState) {
				try {
					const MovableUnit& unit = std::get<MovableUnit>(worldState->getDataConst("Unit"));
					return unit.getResourceTotalCount() > 50;
				}
				catch (const std::bad_variant_access& e) {
					std::cerr << "Erreur: " << e.what() << std::endl;
					return false;
				}
			}
		);
	}
	
	template<>
	Condition createCondition<classicalCondition::FullResources>() {
		return Condition("FullResources",
			[](const std::shared_ptr<WorldState>& worldState) {
				try {
					const MovableUnit& unit = std::get<MovableUnit>(worldState->getDataConst("Unit"));
					return unit.getResourceTotalCount() == 100;
				}
				catch (const std::bad_variant_access& e) {
					std::cerr << "Erreur: " << e.what() << std::endl;
					return false;
				}
			}
		);
	}

	template<>
	Condition createCondition<classicalCondition::NotFullResources>() {
		return Condition("NotFullResources",
			[](const std::shared_ptr<WorldState>& worldState) {
				try {
					const MovableUnit& unit = std::get<MovableUnit>(worldState->getDataConst("Unit"));
					
					return unit.getResourceTotalCount() < 100;
				}
				catch (const std::bad_variant_access& e) {
					std::cerr << "Erreur: " << e.what() << std::endl;
					return false;
				}
			}
		);
	}

	template<>
	Condition createCondition<classicalCondition::OnCity>() {
		return Condition("OnCity",
			[](const std::shared_ptr<WorldState>& worldState) {
				try {
					const MovableUnit& unit = std::get<MovableUnit>(worldState->getDataConst("Unit"));
					const Map& map = std::get<Map>(worldState->getDataConst("GameMap"));
					
					return map.isTileCity(unit.getPos());
				}
				catch (const std::bad_variant_access& e) {
					std::cerr << "Erreur: " << e.what() << std::endl;
					return false;
				}
			}
			);
	}
	
	template<>
	Condition createCondition<classicalCondition::EmptyTile>() {
		return Condition("OnEmptyTile",
			[](const std::shared_ptr<WorldState>& worldState) {
				try {
					const MovableUnit& unit = std::get<MovableUnit>(worldState->getDataConst("Unit"));
					const Map& map = std::get<Map>(worldState->getDataConst("GameMap"));
					return map.isTileFree(unit.getPos());
				}
				catch (const std::bad_variant_access& e) {
					std::cerr << "Erreur: " << e.what() << std::endl;
					return false;
				}
			}
			);
	}

	Condition createConditionNotOnTile(iVec2 target) {
		return Condition("NotOnTile",
			[target](const std::shared_ptr<WorldState>& worldState) {
				try {
					const MovableUnit& unit = std::get<MovableUnit>(worldState->getDataConst("Unit"));
					return unit.getPos() != target;
				}
				catch (const std::bad_variant_access& e) {
					std::cerr << "Erreur: " << e.what() << std::endl;
					return false;
				}
			}
		);
	}

	Condition createConditionOnTile(iVec2 target) {
		return Condition("OnTile",
			[target](const std::shared_ptr<WorldState>& worldState) {
				try {
					const MovableUnit& unit = std::get<MovableUnit>(worldState->getDataConst("Unit"));
					return unit.getPos() == target;
				}
				catch (const std::bad_variant_access& e) {
					std::cerr << "Erreur: " << e.what() << std::endl;
					return false;
				}
			}
		);
	}
};

class FactoryEffect {
	FactoryEffect() = default;
	static FactoryEffect factory;

public:
	FactoryEffect& operator=(const FactoryEffect&) = delete;
	FactoryEffect(const FactoryEffect&) = delete;

	static FactoryEffect& getInstance() noexcept {
		return factory;
	}

	Effect createEffectBuilder() {
		return Effect("BuildCity",
			[](std::shared_ptr<WorldState>& worldState) {
				try {
					MovableUnit& unit = std::get<MovableUnit>(worldState->getData("Unit"));
					std::vector<iVec2>& cities = std::get<std::vector<iVec2>>(worldState->getData("BuildedCity"));
					
					unit.setResources(RessourceCounts(0,0,0));
					cities.push_back(unit.getPos());
				}
				catch (const std::bad_variant_access& e) {
					std::cerr << "Erreur: " << e.what() << std::endl;
				}
			}
		);
	}

	Effect createEffectSupplier() {
		return Effect("Refuel",
			[](std::shared_ptr<WorldState>& worldState) {
				try {
					MovableUnit& unit = std::get<MovableUnit>(worldState->getData("Unit"));
					std::map<iVec2, float>& cities = std::get<std::map<iVec2, float>>(worldState->getData("RefueledCity"));
					const auto& resources = unit.getResources();
					cities[unit.getPos()] = static_cast<float>(std::get<0>(resources) + std::get<1>(resources) * 10 + std::get<2>(resources) * 40);
					unit.setResources(RessourceCounts(0, 0, 0));
				}
				catch (const std::bad_variant_access& e) {
					std::cerr << "Erreur: " << e.what() << std::endl;
				}
			}
		);
	}
		
	Effect createEffectMoveToTile(iVec2 target) {		
		return Effect("MoveOnTile",
			[target](std::shared_ptr<WorldState>& worldState) {
				try {
					MovableUnit& unit = std::get<MovableUnit>(worldState->getData("Unit"));
					unit.setPos(target);
				}
				catch (const std::bad_variant_access& e) {
					std::cerr << "Erreur: " << e.what() << std::endl;
				}
			},
			[target](const std::shared_ptr<WorldState>& ws) {
				try {
					const MovableUnit& unit = std::get<MovableUnit>(ws->getDataConst("Unit"));
					return static_cast<float>(Coordinates::getLengthToTile(unit.getPos(), target));
				}
				catch (const std::bad_variant_access& e) {
					std::cerr << "Erreur: " << e.what() << std::endl;
					return 1000.f;
				}
			}
		);
	}
	
	Effect createEffectMoveToResources(iVec2 target) {		
		return Effect("MoveToResources",
			[target](std::shared_ptr<WorldState>& worldState) {
				try {
					MovableUnit& unit = std::get<MovableUnit>(worldState->getData("Unit"));
					const Map& map= std::get<Map>(worldState->getDataConst("GameMap"));
					iVec2 r = Coordinates::getNearestTileOfType<ResourceTile>(map, target);
					if(Coordinates::getLengthToTile(target, r) > 1) {
						return;
					}
					ResourceTile rt = map.getResourceTile(r);
					const RessourceCounts rc = unit.getResources();
					const auto dispo = 100 - unit.getResourceTotalCount();
					
					switch (rt.type) {
						case ResourceTile::WOOD:
							unit.setResources(RessourceCounts(std::get<0>(rc) + std::min(std::min(80, rt.amount), dispo), std::get<1>(rc), std::get<2>(rc)));
							break;
						case ResourceTile::COAL:
							unit.setResources(RessourceCounts(std::get<0>(rc), std::get<1>(rc) + std::min(std::min(25, rt.amount), dispo), std::get<2>(rc)));
							break;
						case ResourceTile::URANIUM:
							unit.setResources(RessourceCounts(std::get<0>(rc), std::get<1>(rc), std::get<2>(rc) + std::min(std::min(10, rt.amount), dispo)));
							break;
					}
					
					unit.setPos(target);
				}
				catch (const std::bad_variant_access& e) {
					std::cerr << "Erreur: " << e.what() << std::endl;
				}
			},
			[target](const std::shared_ptr<WorldState>& ws) {
				try {
					const MovableUnit& unit = std::get<MovableUnit>(ws->getDataConst("Unit"));
					
					return static_cast<float>(Coordinates::getLengthToTile(unit.getPos(), target)) * 0.9f;;
				}
				catch (const std::bad_variant_access& e) {
					std::cerr << "Erreur: " << e.what() << std::endl;
					return 1000.f;
				}
			}
		);
	}
};

class FactoryAction {
	FactoryAction() = default;
	static FactoryAction factory;
public:

	FactoryAction& operator=(const FactoryAction&) = delete;
	FactoryAction(const FactoryAction&) = delete;

	static FactoryAction& getInstance() noexcept {
		return factory;
	}

	void createMoveToResources(const iVec2& start, const Target& goal, const std::vector<Cluster>& clusters, std::vector<Action>& outActions);
	void createMoveToTile(const iVec2& target, std::vector<Action>& outActions);
	void createRefuelCity(const iVec2& target, std::vector<Action>& outActions);
	void createBuildCity(const iVec2& target, std::vector<Action>& outActions);
};
}