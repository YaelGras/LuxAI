#include <vector>
#ifndef PLANNERGOAP
#define PLANNERGOAP


#include "Action.h"
#include "WorldState.h"
#include "Node.h"
#include <chrono>
namespace Goap {

	class Planner {
	public:
		static std::vector<Action> plan(const WorldState& start, const WorldState& goal, 
			const std::vector<Action>& actions,
			const std::chrono::high_resolution_clock::duration& maxDuration
			);
	};




}
#endif // !PLANNERGOAPs
