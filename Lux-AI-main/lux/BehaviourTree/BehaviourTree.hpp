#ifndef BEHAVIOURTREE
#define BEHAVIOURTREE

#include <memory>
#include <vector>
#include <functional>
#include "lux/Blackboard/Blackboard.hpp"
#include <string>

class Task {
protected:
	std::string name;
public:
	Task() : name{ std::string("Generic Task")} {};
	Task(std::string _name) : name{ _name } {}

	virtual std::string getName() { return name; }
	virtual std::string getName(int depth) { 
		std::string result;
		for (size_t i = 0; i < depth; i++)
		{
			result += "\t";
		}
		return result += "| " + name + "\n";
	}


	virtual bool run(std::shared_ptr<BlackboardData>& data) = 0;
};

//------------------------------------------------------------------------------------------------//
class SequenceTask;
class SelectorTask;

class BehaviourTree : public Task {
	std::shared_ptr<Task> child;
public:
	BehaviourTree() = default;
	BehaviourTree(const std::shared_ptr<Task>& _root)
		: Task(std::string("Behaviour Tree")), child{_root} {}

	BehaviourTree(const std::shared_ptr<Task>& _root, std::string _name) 
		: Task(_name), child{ _root } {}

	static BehaviourTree GetBehaviourTreeLuxAI();
	static SequenceTask& GetBehaviourTreeUnits();
	static SequenceTask& GetBehaviourTreeTiles();
	static SequenceTask& GetBehaviourTreeMove();
	static BehaviourTree GetBehaviourPlanner();


	virtual std::string getName() override;
	virtual std::string getName(int depth) override;
	void DisplayTreeInFile(const std::string& nameFile);

	bool run(std::shared_ptr<BlackboardData>& data) override;
};

// Leaf task take lambda to construct his behaviour
class Leaf : public Task {

	std::function<bool(std::shared_ptr<BlackboardData>&)> behaviour;

public:
	Leaf(const std::function<bool(std::shared_ptr<BlackboardData>&)>& _behaviour)
		: Task(), behaviour{ _behaviour } {
		name = std::string("Generic Leaf");
	}

	Leaf(const std::function<bool(std::shared_ptr<BlackboardData>&)>& _behaviour, std::string &_name)
		: Task(_name),  behaviour{ _behaviour } {}

	bool run(std::shared_ptr<BlackboardData>& data) override {
		if (data->isVerbose())
			std::cerr << name << std::endl;
		return behaviour(data);
	}

};


//------------------------------------------------------------------------------------------------//

class Strategy : public Task {
protected:
	std::vector<std::shared_ptr<Task>> children;

	//Strategy() = default;
public:
	Strategy(const std::vector<std::shared_ptr<Task>>& _children)
		: Task(std::string{ "Generic Strategy" }), children{ _children } {}

	Strategy(const std::vector<std::shared_ptr<Task>>& _children, std::string& _name)
		: Task(_name), children { _children } {}

	void addChild(std::shared_ptr<Task> child) { children.push_back(child); }

	virtual std::string getName() override;

	virtual std::string getName(int depth) override;
};

class SequenceTask : public Strategy {
public:
	SequenceTask(const std::vector<std::shared_ptr<Task>>& _children) 
		: Strategy(_children) {
		name = "Sequence";
	}

	SequenceTask(const std::vector<std::shared_ptr<Task>>& _children, std::string _name)
		: Strategy(_children, _name) {}
	bool run(std::shared_ptr<BlackboardData>& data) override;
};

class SelectorTask : public Strategy {
public:
	SelectorTask(const std::vector<std::shared_ptr<Task>>& _children)
		: Strategy(_children) {
		name = "Selector";
	}
	
	SelectorTask(const std::vector<std::shared_ptr<Task>>& _children, std::string _name)
		: Strategy(_children, _name) {}

	bool run(std::shared_ptr<BlackboardData>& data) override;
};

class ChoiceTaskUnit : public Strategy {
public:
	ChoiceTaskUnit(const std::vector<std::shared_ptr<Task>>& _children)
		: Strategy(_children) {
		name = "Choice Task";
	}

	ChoiceTaskUnit(const std::vector<std::shared_ptr<Task>>& _children, std::string _name)
		: Strategy(_children, _name) {}

	bool run(std::shared_ptr<BlackboardData>& data) override;
	
	virtual std::string getName() override;

	virtual std::string getName(int depth) override;
};
//------------------------------------------------------------------------------------------------//


class Decorator : public Task {
protected:
	std::shared_ptr<Task> child;

public: 
	Decorator(std::shared_ptr<Task> _child) : Task(std::string("Generic Decorator")), child{ _child } {}
	Decorator(std::shared_ptr<Task> _child, std::string _name) : Task(_name), child{ _child } {}

	virtual std::string getName() override;
	virtual std::string getName(int depth) override;
};


class BlackboardManager : Decorator {
public:
	bool run(std::shared_ptr<BlackboardData>& data) override {
		std::shared_ptr<BlackboardData> blackboard{ data };
		bool result = child->run(blackboard);
		return false;
	}
};


class ForEachUnitTask : public Decorator {
public:
	ForEachUnitTask(std::shared_ptr<Task> _child)
		: Decorator(_child, std::string("For Each Unit")) {}

	ForEachUnitTask(std::shared_ptr<Task> _child, std::string& _name)
		: Decorator(_child, _name) {}

	bool run(std::shared_ptr<BlackboardData>& data) override;

};

class ForEachTileTask : public Decorator {
public:
	ForEachTileTask(std::shared_ptr<Task> _child)
		: Decorator(_child, std::string("For Each Tile")) {}

	ForEachTileTask(std::shared_ptr<Task> _child, std::string& _name)
		: Decorator(_child, name) {}

	bool run(std::shared_ptr<BlackboardData>& data) override;
};

class NotTask : public Decorator {
public:
	NotTask(std::shared_ptr<Task> _child)
		: Decorator(_child, std::string("Not")) {}

	NotTask(std::shared_ptr<Task> _child, std::string& _name)
		: Decorator(_child, _name) {}

	bool run(std::shared_ptr<BlackboardData>& data) override {
		return !child->run(data); 
	}
};

#endif 