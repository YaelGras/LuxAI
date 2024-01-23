#pragma once 
#include <map>
#include <queue>
#include <vector>
#include <string>
#include <algorithm>
#include <mutex>

#include "lux/kit.hpp"
struct iVec2;


// ------------------ EXCEPTIONS ------------------ //
struct EEmptyPath {};
struct EEmptyTarget {};

enum TargetType {
	REFUEL = 'R',
	BUILD = 'B',
	MOVE = 'M',
};

struct Target {
	TargetType type;
	iVec2 tile;

	bool operator==(const Target& other) const {
		return type == other.type && tile == other.tile;
	}
	bool operator!=(const Target& other) const {
		return !(*this == other);
	}
};

inline std::ostream& operator<<(std::ostream& os, const Target& target) {
	os << static_cast<char>(target.type) 
		<< " : " << target.tile << "\n";
	return os;
}


// Permet d'éviter les conflits de lecture/écriture sur les données partagées entre les differents threads
struct UnitMutex {
	std::mutex mu_targets;
	std::mutex mu_path;
	std::mutex mu_isRunning;
	std::mutex mu_isAlive;
};

// Classe permettant de stocker les données partagées entre les differents threads tout en assurant la sécurité des données
class DataManager {
	std::map<std::string, std::vector<Target>> m_targets;
	std::map<std::string, std::queue<DIRECTIONS>> m_path;
	std::map<std::string, bool> m_isRunning;
	std::map<std::string, std::thread> m_thread;
	std::map<std::string, bool> m_isAlive;
	std::map<std::string, UnitMutex> m_mutexes;
	std::set<std::string> m_toRun;

	DataManager() = default;
	static DataManager instance;

public:
	bool verbose = false;

	DataManager(const DataManager&) = delete;
	DataManager(DataManager&&) = delete;

	static DataManager& getInstance() {
		return instance;
	}

// ------------------ TARGET ------------------ //
	
	void addTarget(const std::string& id, std::vector<Target>&& targets);

	void addTarget(const std::string& id, const Target& target);

	void resetTarget(const std::string& id, std::vector<Target>&& targets);

	void getAllTargets(std::vector<Target>& outTargets) ;

	void getAllTargets(std::vector<iVec2>& outTargets) ;

	void getAllBuildTargets(std::vector<iVec2>& outTargets);
	
	void getAllRefuelTargets(std::vector<iVec2>& outTargets);

	std::vector<Target> getAllTargets();

	const std::map<std::string, std::vector<Target>>& getAllInfosTargets() const {
		return m_targets;
	}

	const Target& getNextTarget(const std::string& id);

	void removeTarget(const std::string& id, const Target target);

	void doneTarget(const std::string& id);

	bool emptyTargets(const std::string& id);

	bool existingTarget(const Target& target);

	bool existingTargetForOtherUnit(const std::string& id, const Target& target);

// ------------------ PATH ------------------ //

	void addPath(const std::string& id, std::vector<DIRECTIONS>&& path);

	void addPath(const std::string& id, const DIRECTIONS&& path);

	void resetPath(const std::string id, std::vector<DIRECTIONS>&& _path);

	DIRECTIONS getNextMove(const std::string& id);

	const DIRECTIONS& tryNextMove(const std::string& id);

	bool emptyPath(const std::string& id);

// ----------------- RUNNING ----------------- //

	void startThread(const std::string& id) {
		std::lock_guard lock(m_mutexes[id].mu_isRunning);
		m_isRunning[id] = true;
	}

	bool isRunning(const std::string& id) {
		std::lock_guard lock(m_mutexes[id].mu_isRunning);
		return m_isRunning[id];
	}

	void endThread(const std::string& id) {
		std::lock_guard lock(m_mutexes[id].mu_isRunning);
		m_isRunning[id] = false;
	}

	void getAllToRun(std::vector<std::string>& outToRun);

// ------------------ TO RUN ------------------ //

	void addToRun(const std::string& id) {m_toRun.insert(id); }

	void clearToRun() { m_toRun.clear(); }

	std::set<std::string> getAllToRun() { return m_toRun; }

// ------------------ ALIVE ------------------ //

	void setAlive(std::string id);

	void cleanDead();

	void resetAlive();
};
