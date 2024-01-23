#include "WorldState.h"

#include <concepts>
#include <functional>

namespace utils
{
	template<class... Ts>
	struct overloaded : Ts... { using Ts::operator()...; };
	template<class... Ts>
	overloaded(Ts...) -> overloaded<Ts...>;

	struct Visitor {
		template<typename T>
		void operator()(const T& t) const {
			std::cout << t << std::endl;
		}
	};
}

namespace Goap {

	void WorldState::setData(const key_type& key, const value_type& value)
	{		
		data.emplace(key, value);
	}

	const WorldState::value_type& WorldState::getDataConst(const key_type& key) const
	{
		
		if(data.size() > 0) 
			if (auto it = data.find(key); it != data.end())
			{
				return it->second; 
			}
		
		if (previousState)
		{
			return previousState->getDataConst(key);
		}
			
		throw KeyNotFound{};
		
	}

	const WorldState::value_type& WorldState::getData(const key_type& key) const
	{
		return getDataConst(key);
	}
	
	WorldState::value_type& WorldState::getData(const key_type& key)
	{
		if (data.size() > 0) 
			if (auto it = data.find(key); it != data.end())
			return it->second;
		
		if (previousState) {
			const auto prev = previousState->getData(key);
			setData(key, prev);
			return data.at(key);
		} 
		
		throw KeyNotFound{};
	}

	void WorldState::getAllData(std::map<key_type, value_type>& outData) const
	{
		if(data.size() > 0)
			std::ranges::for_each(data, [&outData](const auto& entry) {
				if (!outData.contains(entry.first))
					outData.emplace(entry.first, entry.second);
			});
		if(previousState)
			previousState->getAllData(outData);
	}


	int WorldState::distanceTo(const std::shared_ptr<WorldState>& other) const {
		int dist = 0;

		std::map<key_type, value_type> wsData;
		getAllData(wsData);
		std::map<key_type, value_type> otherData;
		other->getAllData(otherData);

		for (auto entry = wsData.begin(); entry != wsData.end(); ++entry) {

			if (auto it = otherData.find(entry->first);
				it == otherData.end()) {
				dist += 1;
			}
			else {

				int result = std::visit(utils::overloaded{
					[](const std::integral auto& a, const std::integral auto& b) -> int {
						return std::abs(a - b);
					},
					[](const iVec2& a, const iVec2& b) -> int {
						return std::abs(a.first - b.first)
							+ std::abs(a.second - b.second);
					},
					[](const MovableUnit& a, const MovableUnit& b) -> int {
						int dist {};
						if (a.getId() == b.getId()) {
							dist += std::abs(a.getPos().first - b.getPos().first)
								+ std::abs(a.getPos().second - b.getPos().second);
							dist += std::abs(a.getResourceTotalCount() - b.getResourceTotalCount());
						}

						return dist;
					},
					[](const std::map<iVec2, float>& a, const std::map<iVec2, float>& b) -> int {
						int dist = 0;
						for (const auto& entry : a) {
							if (auto it = b.find(entry.first); it != b.end()) {
								if (entry.second < it->second) continue;
								dist += std::max(0, static_cast<int>(entry.second - it->second));
							}
							else {
								dist += static_cast<int>(entry.second);
							}
						}
						return dist;
					},
					[](const std::vector<iVec2>& a, const std::vector<iVec2>& b) -> int {
						int dist = 0;
						for (const auto& entry : a) {
							if (auto it = std::ranges::find(b, entry); it == b.end()) {
								dist++;
							}
						}
						return dist;
					},
					[](const auto&, const auto&) -> int {
						return 0;
					}
					},
					entry->second, it->second
				);

				dist += result;
			}

		}


		return dist;
	}


	int WorldState::distanceTo(const WorldState& other) const {
		int dist = 0;

		std::map<key_type, value_type> wsData;
		getAllData(wsData);
		std::map<key_type, value_type> otherData;
		other.getAllData(otherData);

		for (auto entry = wsData.begin(); entry != wsData.end(); ++entry) {

			if (auto it = otherData.find(entry->first);
				it == otherData.end()) {
				dist += 1;
			} else {

				int result = std::visit(utils::overloaded{
					[](const std::integral auto& a, const std::integral auto& b) -> int {
						return std::abs(a - b);
					},
					[](const iVec2& a, const iVec2& b) -> int {
						return std::abs(a.first - b.first)
							+ std::abs(a.second - b.second);
					},
					[](const MovableUnit& a, const MovableUnit& b) -> int {
						int dist {};
						if (a.getId() == b.getId()) {
							dist += std::abs(a.getPos().first - b.getPos().first)
								+ std::abs(a.getPos().second - b.getPos().second);
							dist += std::abs(a.getResourceTotalCount() - b.getResourceTotalCount());
						}

						return dist;
					},
					[](const std::map<iVec2, float>& a, const std::map<iVec2, float>& b) -> int {
						int dist = 0;
						for(const auto& entry : a) {
							if (auto it = b.find(entry.first); it != b.end()) {
								if (entry.second > it->second) continue;
								dist += std::max(0, static_cast<int>(entry.second - it->second));
							} else {
								dist+= static_cast<int>(entry.second);
							}
						}
						return dist;
					},
					[](const std::vector<iVec2>& a, const std::vector<iVec2>& b) -> int {
						int dist = 0;
						for(const auto& entry : a) {
							if (auto it = std::ranges::find(b, entry); it == b.end()) {
								dist++;
							}
						}
						return dist;
					},
					[](const auto&, const auto&) -> int {
						return 0;
					}
					},
					entry->second, it->second
				);

				dist += result;
			}

		}

		
		return dist;
	}

}
