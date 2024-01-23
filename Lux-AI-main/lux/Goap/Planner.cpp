#include "Planner.h"
#include <algorithm>
#include <set>
#include <vector>
#include <chrono>

namespace Goap {

	struct OperatorSetOpen {
		bool operator()(const std::shared_ptr<Node>& lhs, const std::shared_ptr<Node>& rhs) const {
			if (lhs->estimatedTotalCost() != rhs->estimatedTotalCost())
				return lhs->estimatedTotalCost() < rhs->estimatedTotalCost();
			return lhs->id < rhs->id;
		}
	};
	
	struct OperatorSetClosed {
		bool operator()(const std::shared_ptr<Node>& lhs, const std::shared_ptr<Node>& rhs) const {
			if (lhs->estimatedCostToGoal!= rhs->estimatedCostToGoal)
				return lhs->estimatedCostToGoal < rhs->estimatedCostToGoal;
			else if(lhs->cost != rhs->cost)
				return lhs->cost < rhs->cost;
			return lhs->id < rhs->id;
		}
	};

	std::vector<Action> Planner::plan(const WorldState& start, const WorldState& goal, const std::vector<Action>& actions,
		const std::chrono::high_resolution_clock::duration& maxDuration) {
		//std::cerr << "Start planning\n";
		auto begin = std::chrono::high_resolution_clock::now();

		if (start.successGoal(goal)) {
			return std::vector<Action>();
		}

		std::set<std::shared_ptr<Node>, OperatorSetOpen> open {};  
		std::set<std::shared_ptr<Node>, OperatorSetClosed> closed {};
		std::shared_ptr<WorldState> startPtr = std::make_shared<WorldState>(start);
		std::shared_ptr<Node> startNode = std::shared_ptr<Node>(new Node(-1, startPtr, 0, static_cast<float>(goal.distanceTo(start)), nullptr));
		open.insert(startNode);

		std::shared_ptr<Node> lastNode;
		bool pathFound = false;
		do {
			const std::shared_ptr<Node> current_node = *open.begin();

			if(current_node->action != nullptr)
			
			if (current_node->ws->successGoal(goal)) {
				pathFound = true;
				lastNode = current_node;
				break;
			}

			for (const auto& potential_action : actions) {
				
				if (!potential_action.canRun(current_node->ws)) {
					continue;
				}

				float nextCost = current_node->cost + potential_action.getCost(current_node->ws);
				 
				auto NextWS = potential_action.safeRun(current_node->ws);
				if (!NextWS)
					continue;


				if (auto nodeOpen = std::find_if(open.begin(), end(open),
					[&NextWS](const auto& n) { return n->ws == NextWS.value(); });
					nodeOpen != end(open)) {
					if (nextCost < (*nodeOpen)->cost) {
						(*nodeOpen)->parent_id = current_node->id;
						(*nodeOpen)->cost = nextCost;
						(*nodeOpen)->estimatedCostToGoal = static_cast<float>(goal.distanceTo(NextWS.value()));
						(*nodeOpen)->action = &potential_action;
					}

				}
				else if (auto nodeClosed = std::find_if(closed.begin(), end(closed),
					[&NextWS](const std::shared_ptr<Node>& n) { return n->ws == NextWS.value(); });
					nodeClosed != end(closed)) {
					if (nextCost < (*nodeClosed)->cost) {
						(*nodeClosed)->parent_id = current_node->id;
						(*nodeClosed)->cost = nextCost;
						(*nodeClosed)->estimatedCostToGoal = static_cast<float>(goal.distanceTo(NextWS.value()));
						(*nodeClosed)->action = &potential_action;

						open.insert(*nodeClosed); 
						
						closed.erase(nodeClosed);
					}
				}
				else {
					
					auto newNode = std::shared_ptr<Node>(new Node(current_node->id, NextWS.value(),
						nextCost, static_cast<float>(goal.distanceTo(NextWS.value())),
						&potential_action));
					open.insert(newNode);
				}
			}
			// All Action were added in the correct list, we can remove the current node
			closed.insert(current_node);
			
			open.erase(open.find(current_node));
			
			auto now = std::chrono::high_resolution_clock::now();
			if (std::chrono::duration_cast<std::chrono::microseconds>(now - begin) >
				std::chrono::duration_cast<std::chrono::microseconds>(maxDuration)) {
				std::cerr << "Timeout\n";
				break;
			}
		} while (!open.empty());

		if(closed.empty()) return std::vector<Action>();

		if (!pathFound) {
			std::cerr << "No plan found\n";
			lastNode = *closed.begin();				
		}

		std::vector<Action> plan;
		do {
			if (!lastNode || !lastNode->action) return plan;
			plan.push_back(*lastNode->action);
			auto itr = std::find_if(open.begin(), end(open),
				[&lastNode](const std::shared_ptr<Node>& n) { return n->id == lastNode->parent_id; });
			if (itr == end(open)) {
				itr = std::find_if(closed.begin(), end(closed),
					[&lastNode](const std::shared_ptr<Node>& n) { return n->id == lastNode->parent_id; });
			}
			lastNode = *itr;
		} while (lastNode->parent_id != -1);
		//std::cerr << "End planning\n";

		
		return plan;
	}
}