#include "FactoryAction.h"
#include <lux/World/InfluenceMap.h>
namespace Goap {
	FactoryCondition FactoryCondition::factory;
	FactoryEffect FactoryEffect::factory;
	FactoryAction FactoryAction::factory;
	
	
	void FactoryAction::createMoveToResources(const iVec2& start, const Target& goal, const std::vector<Cluster>& clusters, std::vector<Action>& outActions)
	{
		FactoryEffect& fe = FactoryEffect::getInstance();
		FactoryCondition& fc = FactoryCondition::getInstance();
		if(goal.type == TargetType::BUILD)
		{
			int minX = std::min(start.x, goal.tile.x) - 1;
			int maxX = std::max(start.x, goal.tile.x) + 1;
			int minY = std::min(start.y, goal.tile.y) - 1;
			int maxY = std::max(start.y, goal.tile.y) + 1;

			int distance = Coordinates::getLengthToTile(start, goal.tile);

			for (const auto& _cluster : clusters) {
				for (const auto& border : _cluster.getBorder()) {
					if (border.tilePos.x < minX || border.tilePos.x > maxX || border.tilePos.y < minY || border.tilePos.y > maxY)
					{
						int distanceWithResources = Coordinates::getLengthToTile(start, border.tilePos) + Coordinates::getLengthToTile(border.tilePos, goal.tile);
						if (static_cast<float>(distanceWithResources) * 0.7f <= static_cast<float>(distance) + 2.f) {

							Condition moveCond = fc.createCondition<classicalCondition::NotFullResources>();
							//Condition notOnTile = fc.createConditionNotOnTile(border.tilePos);
							Effect moveEffect = fe.createEffectMoveToResources(border.tilePos);
							outActions.emplace_back(Action(
								std::string("Move to resources : ") + std::to_string(border.tilePos.x) + std::string(" ") + std::to_string(border.tilePos.y),
								{ moveCond },
								{ moveEffect },
								Target{ TargetType::MOVE, border.tilePos }
							));
						}
					}
				}
			}
		}
		else if (goal.type == TargetType::REFUEL) {
			auto& influence = InfluenceMap::getInstance();
			const auto& influenceMap = influence.getInfluenceResFuel();

			struct res {
				iVec2 tilePos;
				int distance;
				int influence;
				bool operator<(const res& other) const
				{
					return static_cast<float>(influence) / static_cast<float>(distance)
						< static_cast<float>(other.influence) / static_cast<float>(other.distance);
				}
			};
			std::priority_queue<res> resourcesQueue;
			for (const auto& _cluster : clusters) {
				for (const auto& resources : _cluster.getResources()) {
					
					int distanceWithResources = Coordinates::getLengthToTile(start, resources.tilePos) + Coordinates::getLengthToTile(resources.tilePos, goal.tile);
					if (influenceMap.at(resources.tilePos) >= 20) {
						
						resourcesQueue.push({ resources.tilePos, distanceWithResources, influenceMap.at(resources.tilePos) });
					}					
				}
			}
			int nbAction = 0;
			while (!resourcesQueue.empty()) {
				if(nbAction > 5)
				{
					break;
				}
				Condition moveCond = fc.createCondition<classicalCondition::NotFullResources>();
				//Condition notOnTile = fc.createConditionNotOnTile(border.tilePos);
				Effect moveEffect = fe.createEffectMoveToResources(resourcesQueue.top().tilePos);
				outActions.emplace_back(Action(
					std::string("Move to resources : ") + std::to_string(resourcesQueue.top().tilePos.x) + std::string(" ") + std::to_string(resourcesQueue.top().tilePos.y),
					{ moveCond },
					{ moveEffect },
					Target{ TargetType::MOVE, resourcesQueue.top().tilePos }
				));
				resourcesQueue.pop();
				nbAction++;
			}
			
		}
	}
	void FactoryAction::createMoveToTile(const iVec2& target, std::vector<Action>& outActions)
	{
		FactoryEffect& fe = FactoryEffect::getInstance();
		FactoryCondition& fc = FactoryCondition::getInstance();
		Condition notOnTile = fc.createConditionNotOnTile(target);
		Effect moveEffect = fe.createEffectMoveToTile(target);
		outActions.emplace_back(Action(
			std::string("Move to tile : ") + std::to_string(target.x) + std::string(" ") + std::to_string(target.y),
			{notOnTile},
			{ moveEffect },
			Target{ TargetType::MOVE, target }
		));
	}
	void FactoryAction::createRefuelCity(const iVec2& target, std::vector<Action>& outActions)
	{

		FactoryEffect& fe = FactoryEffect::getInstance();
		FactoryCondition& fc = FactoryCondition::getInstance();
		Condition onTile = fc.createConditionOnTile(target);
		Condition onCity = fc.createCondition<classicalCondition::OnCity>();
		Condition HaveResources = fc.createCondition<classicalCondition::HaveResources>();
		Effect refuelCity = fe.createEffectSupplier();
		outActions.emplace_back(Action(
			std::string("Refuel city"),
			{onTile, onCity, HaveResources},
			{ refuelCity },
			Target{ TargetType::REFUEL, target }
		));
	}

	void FactoryAction::createBuildCity(const iVec2& target, std::vector<Action>& outActions)
	{
		FactoryEffect& fe = FactoryEffect::getInstance();
		FactoryCondition& fc = FactoryCondition::getInstance();

		Condition onTile = fc.createConditionOnTile(target);

		Condition full = fc.createCondition<classicalCondition::FullResources>();
		Effect build = fe.createEffectBuilder();
		outActions.emplace_back(Action(
			std::string("Build city"),
			{ onTile, full },
			{ build },
			Target{ TargetType::BUILD, target }
		));

	}
}