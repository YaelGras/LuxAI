#include "BehaviourTree.hpp"
#include "FactoryLeaf.hpp"
#include <fstream>

// ------------------------------ Behaviour Tree ----------------------------------
BehaviourTree BehaviourTree::GetBehaviourTreeLuxAI()
{
	/*
	* Need in blackboard :
	*	Map GameMap
	*	std::vector<std::string> Orders 
	*	std::unordered_map<std::string, MovableUnit> Units
	*	std::set<pair> unitpos UnitsPos
	* 
	* Sch�ma
	* Root 
	*	| Sequence :
	*	|	| ForEachUnit :
	*	|	|	| BehaviourTree Unit
	*	|	| ForEachTile :
	*	|	|	| BehaviourTree Tile
	* End
	*/

	std::shared_ptr<Task> BTUnit = 
		std::make_shared<SequenceTask>(GetBehaviourTreeUnits());
	std::shared_ptr<Task> BTTiles = 
		std::make_shared<SequenceTask>(GetBehaviourTreeTiles());

	std::shared_ptr<Task> FEUT = 
		std::make_shared<ForEachUnitTask>(BTUnit);
	std::shared_ptr<Task> FETT =
		std::make_shared<ForEachTileTask>(BTTiles);


	std::vector<std::shared_ptr<Task>> sequenceChildren{ FETT, FEUT };
	std::shared_ptr<Task> GlobalSequence =
		std::make_shared<SequenceTask>(sequenceChildren);

	static BehaviourTree BTLuxAI(GlobalSequence, std::string("Behaviour Tree Unan"));

	BTLuxAI.DisplayTreeInFile(std::string("BehaviourTree.txt"));
	return BTLuxAI;

}

SequenceTask& BehaviourTree::GetBehaviourTreeUnits()
{
	FactoryLeaf& factoryLeaf = FactoryLeaf::getInstance();

	// BuildCity
	/*
	* Need in blackboard :
	*	MovableUnit Unit
	*	Map GameMap
	*	string Order
	*
	* Sch�ma
	* Sequence :
	*	| Full ?
	*	| emptyTileUnderUnit ?
	*	| BuildCity
	* End
	*/

	std::shared_ptr<Task> full =
		std::make_shared<Leaf>(
			factoryLeaf.create<classicalLeaf::fullResources_t>());
	
	std::shared_ptr<Task> onTarget = 
		std::make_shared<Leaf>(
			factoryLeaf.create<classicalLeaf::onTargetTile_t>());

	std::shared_ptr<Task> emptyTile =
		std::make_shared<Leaf>(
			factoryLeaf.create<classicalLeaf::emptyTileUnderUnit_t>());
	
	std::shared_ptr<Task> BuildCity =
		std::make_shared<Leaf>(
			factoryLeaf.create<classicalLeaf::buildCity_t>());

	std::vector<std::shared_ptr<Task>> sequenceBuildCity { full, onTarget, emptyTile, BuildCity };
	std::shared_ptr<Task> BuildCitySequence =
		std::make_shared<SequenceTask>(sequenceBuildCity);


	
	// BTUnit
	/*
	* Need in blackboard :
	*	MovableUnit Unit
	*	Map GameMap
	*	string Order
	*	float Cooldown
	*
	* Sch�ma
	* Sequence :
	* | Declare Alive
	* | Cooldown ?
	* | Option :
	*	| BuildCity
	*	| Move
	* End
	*/
	std::shared_ptr<Task> DeclareAlive =
		std::make_shared<Leaf>(
			factoryLeaf.create<classicalLeaf::declareAlive_t>());
	
	std::shared_ptr<Task> Cooldown =
		std::make_shared<Leaf>(
			factoryLeaf.create<classicalLeaf::cooldown_t>());
	std::shared_ptr<Task> BTMove = std::make_shared<SequenceTask>(GetBehaviourTreeMove());

	std::vector<std::shared_ptr<Task>> optionUnit{ BuildCitySequence, 
		BTMove };
	std::shared_ptr<Task> Option =
		std::make_shared<SelectorTask>(optionUnit);

	std::vector<std::shared_ptr<Task>> sequenceUnit{ DeclareAlive, Cooldown, Option };
	std::shared_ptr<Task> BTUnit =
		std::make_shared<SequenceTask>(sequenceUnit);
	
	static SequenceTask UnitSequence (sequenceUnit); 
	return UnitSequence;
}

SequenceTask& BehaviourTree::GetBehaviourTreeTiles()
{
	FactoryLeaf& factoryLeaf = FactoryLeaf::getInstance();

	// BuildWorker
	/*
	* Need in blackboard :
	*	std::unordered_map<std::string, MovableUnit> Units
	*	Map GameMap
	*	string Order
	*	CityTile CityTile
	* 
	* Sch�ma
	* Sequence :
	* | FullUnit ?
	* | BuildWorker
	* End
	*/

	std::shared_ptr<Task> FullUnit =
		std::make_shared<Leaf>(
			factoryLeaf.create<classicalLeaf::notFullUnit_t>());

	std::shared_ptr<Task> BuildWorker =
		std::make_shared<Leaf>(
			factoryLeaf.create<classicalLeaf::buildWorker_t>());

	std::vector<std::shared_ptr<Task>> sequenceBuildWorker{ FullUnit, BuildWorker };
	std::shared_ptr<Task> BuildWorkerSequence =
		std::make_shared<SequenceTask>(sequenceBuildWorker);

	// Research
	/*
	* Need in blackboard :
	*	string Order
	*	CityTile CityTile
	*
	* Sch�ma
	* Sequence :
	* | FullResearchPoint ?
	* | Research
	* End
	*/

	std::shared_ptr<Task> NotFullResearchPoint =
		std::make_shared<Leaf>(
			factoryLeaf.create<classicalLeaf::notFullResearchPoint_t>());
	std::shared_ptr<Task> Research =
		std::make_shared<Leaf>(
			factoryLeaf.create<classicalLeaf::research_t>());


	std::vector<std::shared_ptr<Task>> sequenceResearch{ NotFullResearchPoint, Research };
	std::shared_ptr<Task> ResearchSequence =
		std::make_shared<SequenceTask>(sequenceResearch);

	// BTTile
	/*
	* Need in blackboard :
	*	std::unordered_map<std::string, MovableUnit> Units
	*	Map GameMap
	*	string Order
	*	CityTile CityTile
	*	float Cooldown
	*
	* Sch�ma
	* Sequence :
	* | Cooldown ?
	* | Option :
	*	| BuildWorker
	*	| Research
	* End
	*/
	
	std::shared_ptr<Task> Cooldown =
		std::make_shared<Leaf>(
			factoryLeaf.create<classicalLeaf::cooldown_t>());

	std::vector<std::shared_ptr<Task>> optionTile{ BuildWorkerSequence, ResearchSequence };
	std::shared_ptr<Task> Option =
		std::make_shared<SelectorTask>(optionTile);

	std::vector<std::shared_ptr<Task>> sequenceTile{ Cooldown, Option };

	static SequenceTask TileSequence(sequenceTile);
	return TileSequence;
}

SequenceTask& BehaviourTree::GetBehaviourTreeMove()
{

	FactoryLeaf& factoryLeaf = FactoryLeaf::getInstance();

	// MoveUnit
	/*
	* Need in blackboard :
	*	MovableUnit Unit
	*	Map GameMap
	*	string Order
	*
	* Sch�ma
	* Sequence : Move
	*	| MajTarget
	*	| Choices : Get New Objectif
	*		| HaveTarget
	*			| Have Path	
	*				| Follow Path 
	*			| !Have Path
					| Find Path
					| Follow Path
			| !Have Target
				| Get Target
				| Find Path
				| Follow Path
	*/


	std::shared_ptr<Task> majTarget = std::make_shared<Leaf>(
		factoryLeaf.create<classicalLeaf::majTarget_t>());
	

	std::shared_ptr<Task> havePath = std::make_shared<Leaf>(
		factoryLeaf.create<classicalLeaf::pathAvailable_t>());

	std::shared_ptr<Task> followPath = std::make_shared<Leaf>(
		factoryLeaf.create<classicalLeaf::pathFollower_t>());

	std::shared_ptr<Task> getTarget = std::make_shared<Leaf>(
		factoryLeaf.create<classicalLeaf::getTarget_t>());
	
	std::shared_ptr<Task> planTarget = std::make_shared<Leaf>(
		factoryLeaf.create<classicalLeaf::planTarget_t>());
	
	std::shared_ptr<Task> findPath = std::make_shared<Leaf>(
		factoryLeaf.create<classicalLeaf::pathFinder_t>());

	std::shared_ptr<Task> move = std::make_shared<Leaf>(
		factoryLeaf.create<classicalLeaf::move_t>());

	std::vector<std::shared_ptr<Task>> follow = 
		{ followPath, move };

	std::shared_ptr<Task> FollowSequence = 
		std::make_shared<SequenceTask>(follow, std::string("Sequence : Follow "));

	std::vector<std::shared_ptr<Task>> choice =
		{ FollowSequence, findPath, getTarget };

	std::shared_ptr<Task> ChoiceTask =
		std::make_shared<ChoiceTaskUnit>(choice, std::string("ChoiceUnitTask : Move"));


	std::vector<std::shared_ptr<Task>> sequence =
	{ majTarget, ChoiceTask };

	
	
	static SequenceTask MoveOption(sequence, std::string("Sequence : Move"));

	
	return MoveOption;
}

BehaviourTree BehaviourTree::GetBehaviourPlanner()
{
	FactoryLeaf& factoryLeaf = FactoryLeaf::getInstance();

	// GOAP,
	// AStar
	// FollowPath
	// Move

	std::shared_ptr<Task> GOAP = std::make_shared<Leaf>(
		factoryLeaf.create<classicalLeaf::planTarget_t>());
	std::shared_ptr<Task> AStar = std::make_shared<Leaf>(
		factoryLeaf.create<classicalLeaf::pathFinder_t>());
	std::shared_ptr<Task> Follower = std::make_shared<Leaf>(
		factoryLeaf.create<classicalLeaf::pathFollower_t>());
	std::shared_ptr<Task> Move = std::make_shared<Leaf>(
		factoryLeaf.create<classicalLeaf::move_t>());

	
	std::vector<std::shared_ptr<Task>> sequence =
		{ GOAP, AStar, Follower, Move };

	std::shared_ptr<Task> Planner =
		std::make_shared<SequenceTask>(sequence, std::string("Sequence : Follow "));


	return BehaviourTree{ Planner };
}

bool BehaviourTree::run(std::shared_ptr<BlackboardData>& data)
{

	return child->run(data);
}

std::string BehaviourTree::getName()
{
	std::string out;
	out += name + "\n";
	out += child->getName(1);
	return out;
}

std::string BehaviourTree::getName(int depth)
{
	std::string out;
	for (size_t i = 0; i < depth; i++)
	{
		out += "\t";
	}
	out += "| " + name + "\n";
	out += child->getName(depth + 1);

	return out;
}

void BehaviourTree::DisplayTreeInFile(const std::string& nameFile)
{
	std::ofstream file(nameFile);

	file << getName();

	file.close();
	
}

// ------------------------------ Strategy ----------------------------------

std::string Strategy::getName() {
	std::string out{ name + "\n" };
	for (std::shared_ptr<Task> child : children)
	{
		out += child->getName(1);
	}
	return out;
}

std::string Strategy::getName(int depth) {
	std::string out;
	for (size_t i = 0; i < depth; i++)
	{
		out += "\t";
	}
	out += name + "\n";
	for (std::shared_ptr<Task> child : children)
	{
		out += child->getName(depth + 1);
	}
	return out;
}

bool SequenceTask::run(std::shared_ptr<BlackboardData>& data)
{
	
	for (std::shared_ptr<Task> task : children) {
		if (!task->run(data))
			return false;
	}
	return true;
}

bool SelectorTask::run(std::shared_ptr<BlackboardData>& data)
{
	for (std::shared_ptr<Task> task : children) {
		if (task->run(data))
			return true;
	}
	return false;
}

bool ForEachUnitTask::run(std::shared_ptr<BlackboardData>& data)
{
	try {
		
		std::vector<std::string>& orders
			= data->getData<std::vector<std::string>>("Orders");

		std::unordered_map<std::string, MovableUnit>& units 
			= data->getData<std::unordered_map<std::string, MovableUnit>>("Units");


		if (units.empty()) return true;
		std::shared_ptr<BlackboardData> childData{ data };
		childData->insertData("Unit", units.begin()->second);
		childData->insertData("Cooldown", 0.0f);
		childData->insertData("Order", std::string{});

		for (auto& [id, unit] : units) {

			// Create child blackboard for running the child with the current tile infos
			
			auto& u = childData->getData<MovableUnit>("Unit");
			u = unit;
			auto& cd = childData->getData<float>("Cooldown");
			cd = unit.getCooldown();

			if(childData->isVerbose())
				std::cerr << "Run unit : " << unit.getId();
			bool success = child->run(childData);


			if (childData->isVerbose())
			{
				std::string s = "blackboard" + unit.getId() + ".txt";
				childData->ecrireBlackboardDansFichier(s, 0);
			}
			if (childData->isVerbose())
				std::cerr << "\n\tSuccess : " << success << std::endl;

			if (success) {
				// Add the order in the parent blackboard
				std::string& _order = childData->getData<std::string>("Order");
				// Ajouter l'rder dans la liste d'ordres
				if (!_order.empty())
				{
					if (childData->isVerbose())
						try
						{
							iVec2 nextTile = childData->getData<iVec2>("NextTile");
							std::cerr << "Unit : " << id << " at tile " << unit.getPos() << " wants to go to " << nextTile << std::endl;
						}
						catch (...) {}
					orders.push_back(_order);
					_order = std::string{};
				}
			}
		}
	return true;

	}
	catch (TypeMismatch&) { return false; }
	catch (KeyNotFound&) { return false; }
}

bool ForEachTileTask::run(std::shared_ptr<BlackboardData>& data)
{
	try {

		// TODO : Change type for tile infos
		Map& m_gameMap = data->getData<Map>("GameMap");

		for (const auto& [p, city] : m_gameMap.getAllCityTiles())
		{
			// Create child blackboard for running the child with the current tile infos
			std::shared_ptr<BlackboardData> childData(data);
			childData->insertData("CityTile", city);
			childData->insertData("Cooldown", city.cooldown);
			childData->insertData("Order", std::string{});

			bool success = child->run(childData);
			if (success) {
				// Add the order in the parent blackboard
				std::string _order = childData->getData<std::string>("Order");
				// Ajouter l'rder dans la liste d'ordres
				if (!_order.empty()) {
					std::vector<std::string>& orders = data->getData<std::vector<std::string>>("Orders");
					orders.push_back(_order);

				}
			}
			
		}
		return true;
	}
	catch (TypeMismatch&) { return false; }
	catch (KeyNotFound&) { return false; }
}

std::string Decorator::getName()
{
	std::string out;
	out += name + "\n";
	out += child->getName(1);
	return out;
}

std::string Decorator::getName(int depth)
{
	std::string out;
	for (size_t i = 0; i < depth; i++)
	{
		out += "\t";
	}
	out += "| " + name + "\n";
	out += child->getName(depth + 1);

	return out;
}

bool ChoiceTaskUnit::run(std::shared_ptr<BlackboardData>& data)
{
	FactoryLeaf& factoryLeaf = FactoryLeaf::getInstance();

	std::shared_ptr<Task> HaveTarget = 
		std::make_shared<Leaf>(
			factoryLeaf.create<classicalLeaf::haveTarget_t>());

	std::shared_ptr<Task> HavePath =
		std::make_shared<Leaf>(
			factoryLeaf.create<classicalLeaf::pathAvailable_t>());

	std::shared_ptr<Task> isRunning =
		std::make_shared<Leaf>(
			factoryLeaf.create<classicalLeaf::isPlanning_t>());

	if(isRunning->run(data))
		return false;

	if (HaveTarget->run(data)) {
		if (HavePath->run(data)) {
			return children[0]->run(data);
		}
		else {
			bool res = children[1]->run(data);
			//if(res)
				return children[0]->run(data);
		}
	}
	else {
		children[2]->run(data);			
	}
	return false;
}

std::string ChoiceTaskUnit::getName()
{
	std::string out = name;
	out += "\n\t | Have Target ";
	out += "\n\t\t | Have Path";
	out += "\n\t\t\t" + children[0]->getName();
	out += "\n\t\t | Not Have Path";
	out += "\n\t\t\t" + children[1]->getName();
	out += "\n\t\t\t" + children[0]->getName();
	out += "\n\t | Not Have Target ";
	out += "\n\t\t" + children[2]->getName();
	out += "\n\t\t" + children[1]->getName();
	out += "\n\t\t" + children[0]->getName();

	return out;
}

std::string ChoiceTaskUnit::getName(int depth)
{
	std::string decalage;
	for (int i = 0; i < depth; i++)
	{
		decalage += "\t";
	}
	std::string out = decalage + name;
	out += "\n\t"+ decalage + " | Have Target ";
	out += "\n\t\t" + decalage + " | Have Path";
	out += "\n\t\t\t"+ decalage + children[0]->getName();
	out += "\n\t\t" + decalage + " | Not Have Path";
	out += "\n\t\t\t" + decalage + children[1]->getName();
	out += "\n\t\t\t" + decalage + children[0]->getName();
	out += "\n\t" + decalage + " | Not Have Target ";
	out += "\n\t\t" + decalage + children[2]->getName();
	out += "\n\t\t" + decalage + children[1]->getName();
	out += "\n\t\t" + decalage + children[0]->getName();
	return out;
}
