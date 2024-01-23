#ifndef NODE
#define NODE
#include <iostream>
#include <memory>
#include "Action.h"
#include "WorldState.h"
namespace Goap {
    struct Node {
        static int next_id; // a static that lets us assign incrementing, unique IDs to nodes

        std::shared_ptr<WorldState> ws;
        int id;
        int parent_id;
        float cost;  // Cost to come to this node
        float estimatedCostToGoal; // Cost to go from this node to the goal
        const Action* action;

        Node();
        Node(int _parent_id, std::shared_ptr<WorldState> _ws, float _cost, float _estimatedCostToGoal, const Action* _action);

        Node(const Node& other) : 
            ws(other.ws), 
            id(other.id), 
            parent_id(other.parent_id), 
            cost(other.cost), 
            estimatedCostToGoal(other.estimatedCostToGoal), 
            action(other.action) {}
        
        Node operator=(const Node& other) {
			ws = other.ws;
			id = other.id;
			parent_id = other.parent_id;
			cost = other.cost;
			estimatedCostToGoal = other.estimatedCostToGoal;
			action = other.action;
			return *this;
		}

        
        // F -- which is simply G+H -- is autocalculated
        float estimatedTotalCost() const {
            return cost + estimatedCostToGoal;
        }

        std::ostream& operator<<(std::ostream& out) {
            out << "Node { \n\tid : " << id
                << "\n\tparent : " << parent_id
                << "\n\ttotalCostToGoal (estimated) : " << estimatedTotalCost()
                << "\n\tcost :" << cost
                << "\n\testimatedCostToGoal :" << estimatedCostToGoal
                << "\n}\n";
            return out;
        }

        bool operator<(const Node* other) const {
            if (estimatedTotalCost() != other->estimatedTotalCost())
                return estimatedTotalCost() < other->estimatedTotalCost();
			return id < other->id;			
		}

        bool operator==(const Node* other) const {
			return id == other->id;
		}
        
        bool operator==(const std::shared_ptr<WorldState>& other) const {
			return ws == other;
		}
        
        bool operator!=(const Node* other) const {
			return !(this == other);
		} 
        
        bool operator<(const Node& other) const {
            if (estimatedTotalCost() != other.estimatedTotalCost())
                return estimatedTotalCost() < other.estimatedTotalCost();
			return id < other.id;			
		}

        bool operator==(const Node& other) const {
			return id == other.id;
		}
        
        bool operator!=(const Node& other) const {
			return !(*this == other);
		}
    };
}

#endif // !NODE
