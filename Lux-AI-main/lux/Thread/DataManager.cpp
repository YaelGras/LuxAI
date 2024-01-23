#include "DataManager.h"
#include "lux/World/Tiles.h"
DataManager DataManager::instance{};



// ------------------------------ TARGETS ----------------------------------


void DataManager::addTarget(const std::string& id, std::vector<Target>&& targets)
{
	std::lock_guard lock(m_mutexes[id].mu_targets);
	if (verbose) std::cerr << "New target for " << id  << std::endl;
	m_targets[id] = targets;
}

void DataManager::addTarget(const std::string& id, const Target& target)
{
	std::lock_guard lock(m_mutexes[id].mu_targets);
	if (verbose) std::cerr << "New target for " << id << " : " << target << std::endl;
	m_targets[id].emplace_back(target);
}

void DataManager::resetTarget(const std::string& id, std::vector<Target>&& targets)
{
	std::lock_guard lock(m_mutexes[id].mu_targets);
	if (verbose) std::cerr << "Reset target for " << id << std::endl;
	m_targets[id] = targets;
}

void DataManager::getAllTargets(std::vector<Target>& outTargets)
{
	if (verbose) std::cerr << "Get all target " << std::endl;
	std::for_each(begin(m_targets), end(m_targets), [&](auto& target) {
		std::lock_guard lock(m_mutexes[target.first].mu_targets);
			for(const auto& t : target.second)
				outTargets.emplace_back(t);
		});

}

void DataManager::getAllTargets(std::vector<iVec2>& outTargets)
{
	if (verbose) std::cerr << "Get all target " << std::endl;
	std::for_each(begin(m_targets), end(m_targets), [&](auto& target) {
		std::lock_guard lock(m_mutexes[target.first].mu_targets);
			for(const auto& t : target.second)
				outTargets.emplace_back(t.tile);
		});

}

void DataManager::getAllBuildTargets(std::vector<iVec2>& outTargets)
{
	if (verbose) std::cerr << "Get all build target " << std::endl;
	std::for_each(begin(m_targets), end(m_targets), [&](auto& target) {
		std::lock_guard lock(m_mutexes[target.first].mu_targets);
		for (const auto& t : target.second)
			if(t.type == TargetType::BUILD)
				outTargets.emplace_back(t.tile);
		});
}

void DataManager::getAllRefuelTargets(std::vector<iVec2>& outTargets)
{
	if (verbose) std::cerr << "Get all Refuel target " << std::endl;
	std::for_each(begin(m_targets), end(m_targets), [&](auto& target) {
		std::lock_guard lock(m_mutexes[target.first].mu_targets);
		for (const auto& t : target.second)
			if (t.type == TargetType::REFUEL)
			{
				outTargets.emplace_back(t.tile);
			}
		});

}

std::vector<Target> DataManager::getAllTargets()
{
	std::vector<Target> outTargets;
	getAllTargets(outTargets);
	return outTargets;
}

const Target& DataManager::getNextTarget(const std::string& id)
{
	std::lock_guard lock(m_mutexes[id].mu_targets);
	if (verbose) std::cerr << "Get next target for " << id << std::endl;
	if(m_targets[id].empty())
		throw EEmptyTarget {};

	return m_targets[id][0];
}

void DataManager::removeTarget(const std::string& id, const Target target)
{
	std::lock_guard lock(m_mutexes[id].mu_targets);
	if (verbose) std::cerr << "Remove target for " << id << " : " << target << std::endl;
	if(auto it = std::ranges::find(m_targets[id], target); it != m_targets[id].end())
	{
		
		m_targets[id].erase(it);
	}
}

void DataManager::doneTarget(const std::string& id)
{
	std::lock_guard lock(m_mutexes[id].mu_targets);
	if (verbose) std::cerr << "Done target for " << id << std::endl;
	if (!m_targets[id].empty())
	{
		
		m_targets[id].erase(m_targets[id].begin()); 
	}
}

bool DataManager::emptyTargets(const std::string& id)
{
	std::lock_guard lock(m_mutexes[id].mu_targets);

	if (verbose) std::cerr << "Empty target for " << id << std::endl;
	return m_targets[id].empty();
}

bool DataManager::existingTarget(const Target& target)
{
	return std::any_of(begin(m_targets), end(m_targets), [&](const auto& entry) {
		std::lock_guard lock(m_mutexes[entry.first].mu_targets);
		return std::ranges::find(entry.second, target) != entry.second.end();
		});
}

bool DataManager::existingTargetForOtherUnit(const std::string& id, const Target& target)
{
	return std::any_of(begin(m_targets), end(m_targets), [&](const auto& entry) {
		if(entry.first == id)
			return false;
		std::lock_guard lock(m_mutexes[entry.first].mu_targets);
		return std::ranges::find(entry.second, target) != entry.second.end();
		});
}


// ------------------------------- PATH ------------------------------------

void DataManager::addPath(const std::string& id, std::vector<DIRECTIONS>&& path)
{
	std::lock_guard lock(m_mutexes[id].mu_path);
	if (verbose) std::cerr << "Add path for " << id  << std::endl;
	std::for_each(begin(path), end(path), [&](auto dir) {
		m_path[id].push(dir);
		});


}

void DataManager::addPath(const std::string& id, const DIRECTIONS&& path)
{
	std::lock_guard lock(m_mutexes[id].mu_path);
	if (verbose) std::cerr << "Add path for " << id << std::endl;
	m_path[id].push(path);
}

void DataManager::resetPath(const std::string id, std::vector<DIRECTIONS>&& _path)
{
	std::lock_guard lock(m_mutexes[id].mu_path);
	if (verbose) std::cerr << "Add path for " << id << std::endl;
	m_path[id] = std::queue<DIRECTIONS>();
	std::for_each(begin(_path), end(_path), [&](auto dir) {
		m_path[id].push(dir);
		});

}

DIRECTIONS DataManager::getNextMove(const std::string& id)
{
	std::lock_guard lock(m_mutexes[id].mu_path);
	if (verbose) std::cerr << "Get next move for " << id << std::endl;
	if (!m_path.contains(id) || m_path[id].empty())
		throw EEmptyPath{};
	DIRECTIONS next = m_path[id].front();
	m_path[id].pop();
	return next;
}

const DIRECTIONS& DataManager::tryNextMove(const std::string& id)
{
	std::lock_guard lock(m_mutexes[id].mu_path);
	if (verbose) std::cerr << "Try next move for " << id << " size " << static_cast<char>(m_path[id].front()) << std::endl;
	if (!m_path.contains(id) || m_path[id].empty())
		throw EEmptyPath{};
	return m_path[id].front();
}

bool DataManager::emptyPath(const std::string& id)
{
	std::lock_guard lock(m_mutexes[id].mu_path);
	if (verbose) std::cerr << "Empty path for " << id << std::endl;
	return m_path[id].empty();
}


// ------------------------------- IS RUNNING ------------------------------------

void DataManager::getAllToRun(std::vector<std::string>& outToRun)
{
	if (verbose) std::cerr << "Get all to run " << std::endl;
	// Récupérer l'ensemble des element qui n'ont plus de targets et plus de path
	std::for_each(begin(m_targets), end(m_targets), [&](const auto& target) {
		std::lock_guard lock_t(m_mutexes[target.first].mu_targets);
		std::lock_guard lock_p(m_mutexes[target.first].mu_path);
		std::lock_guard lock_r(m_mutexes[target.first].mu_isRunning);
		if (target.second.empty() && m_path[target.first].empty() && !m_isRunning[target.first])
			outToRun.push_back(target.first);
		});

}
// ------------------------------- IS ALIVE --------------------------------------


void DataManager::setAlive(std::string id)
{
	std::lock_guard lock(m_mutexes[id].mu_isAlive);
	m_isAlive[id] = true;
}

void DataManager::cleanDead()
{

	if (verbose) std::cerr << "Clean dead"  << std::endl;
	auto it = m_isAlive.begin();
	while (it != m_isAlive.end())
	{
		const auto& entry = *it;

		if (!entry.second)
		{
			std::lock_guard lock_t(m_mutexes[entry.first].mu_targets);
			std::lock_guard lock_p(m_mutexes[entry.first].mu_path);
			std::lock_guard lock_r(m_mutexes[entry.first].mu_isRunning);

			m_targets.erase(entry.first);
			m_path.erase(entry.first);

			it = m_isAlive.erase(it); 
		}
		else
		{
			++it; 
		}
	}
}

void DataManager::resetAlive()
{
	if (verbose) std::cerr << "Reset running" << std::endl;

	for (auto& entry : m_isAlive)
	{
		std::lock_guard lock_r(m_mutexes[entry.first].mu_isRunning);
		entry.second = false;
	}
}
