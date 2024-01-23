#include "Action.h"
#include <algorithm>
#include "WorldState.h"
namespace Goap {
	bool Goap::Action::canRun(const std::shared_ptr<WorldState>& ws) const
	{
		return std::all_of(
			begin(preconditions),
			end(preconditions),
			[&ws](const Condition& c) { return c(ws); });;
	}

	bool Action::run(std::shared_ptr<WorldState>& ws) const
	{
		try {
			for (Effect e : effects)
				e(ws);

			return true;
		}
		catch (...) { return false; }
	}

	std::optional<std::shared_ptr<WorldState>> Action::safeRun(const std::shared_ptr<WorldState>& ws) const
	{
		std::shared_ptr<WorldState> newWS = WorldState::createChild(ws);
		bool success = run(newWS);
		if (!success) return {};
		return { newWS };
	}	
}
