#pragma once
#include <thread>
#include <vector>

class ThreadManager
{
	std::vector<std::thread> m_threads;

	ThreadManager() = default;
	~ThreadManager() = default;

public:
	static ThreadManager& getInstance() {
		static ThreadManager instance;
		return instance;
	}

	ThreadManager(ThreadManager const&) = delete;
	void operator=(ThreadManager const&) = delete;

	void addThread(std::thread&& thread) {
		m_threads.push_back(std::move(thread));
	}

	void joinAll() {
		for (auto& thread : m_threads) {
			if(thread.joinable())
				thread.join();
		}
	}
};