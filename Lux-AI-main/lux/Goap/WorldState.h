#ifndef WORLDSTATE
#define WORLDSTATE
#include "lux/Blackboard/Blackboard.hpp"
#include <memory>
#include <string>
#include <map>
#include <variant>


#include "lux/World/Tiles.h"
#include "lux/World/Unit.h"

namespace Goap {

	class WorldState {
		using value_type = std::variant<
			MovableUnit,
			Map,
			//Pour savoir si on a refuel ou pas
			std::map<iVec2, float>,
			//Pour savoir ou on a build
			std::vector<iVec2>>;
		using key_type = std::string;
		std::map<key_type, value_type> data{};
		std::shared_ptr<WorldState> previousState = nullptr;
		
		public:
			WorldState() = default;

			static std::shared_ptr<WorldState> createChild(const std::shared_ptr<WorldState>& other) {
				std::shared_ptr<WorldState> child = std::make_shared<WorldState>();
				child->previousState = other;
				return child;
			}

			void setData(const key_type& key, const value_type& value);
						
			[[nodiscard]] const WorldState::value_type& getDataConst(const key_type& key) const;

			[[nodiscard]] const WorldState::value_type& getData(const key_type& key) const;

			[[nodiscard]] WorldState::value_type& getData(const key_type& key);

			[[nodiscard]] void getAllData(std::map<key_type, value_type>& outData) const ;

			int distanceTo(const std::shared_ptr<WorldState>& other) const;

			int distanceTo(const WorldState& other) const;

			bool operator==(const std::shared_ptr<WorldState>& other) const {
				return distanceTo(other) == 0 && other->distanceTo(*this) == 0;
			}
			
			bool operator==(const WorldState& other) const {
				return distanceTo(other) == 0 && other.distanceTo(*this) == 0;
			}

			bool successGoal(const std::shared_ptr<WorldState>& goal) const {
				return  goal->distanceTo(*this) == 0;
			}

			bool successGoal(const WorldState& goal) const {
				return  goal.distanceTo(*this) == 0;
			}

		};

}


#endif // !WORLDSTATE
