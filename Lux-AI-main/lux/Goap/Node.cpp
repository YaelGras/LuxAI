#include "Node.h"
#include <iostream>

namespace Goap {
    int Node::next_id = 0;

    Node::Node() 
        : id{ ++next_id },
        parent_id{ -1 },
        cost{ 0 },
        estimatedCostToGoal{ 0 },
        action{nullptr} {}

    Node::Node(int _parent_id, std::shared_ptr<WorldState> _ws, float _cost, float _estimatedCostToGoal, const Action* _action)
        : id{ next_id++ },
        parent_id{ _parent_id },
        ws { _ws },
        cost{ _cost },
        estimatedCostToGoal{ _estimatedCostToGoal },
        action{ _action } {}
}