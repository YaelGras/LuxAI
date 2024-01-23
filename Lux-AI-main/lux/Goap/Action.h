
#ifndef GOAPACTION
#define GOAPACTION

#include <vector>
#include <string>
#include <functional>
#include <iostream>
#include <optional>
#include "lux/Thread/DataManager.h"
namespace Goap {
    class WorldState;

    class Condition {
        std::string name;
        std::function<bool(const std::shared_ptr<WorldState>&)> condition;
    public:
        Condition(std::string _name, std::function<bool(const std::shared_ptr<WorldState>&)> _condition)
			: name(_name), condition(_condition) {}

        bool operator()(const std::shared_ptr<WorldState>& worldState) const {
			return condition(worldState);
		}

        [[nodiscard]]std::string getName() const { return name; }
    };

    class Effect {
        std::string name;
        std::function<void(std::shared_ptr<WorldState>&)> effect;
        std::function<float(const std::shared_ptr<WorldState>&)> cost = [](const std::shared_ptr<WorldState>&) { return 1.f; };
    public:
        Effect(std::string _name, std::function<void(std::shared_ptr<WorldState>&)> _effect)
            : name(_name), effect(_effect) {}
        
        Effect(std::string _name, std::function<void(std::shared_ptr<WorldState>&)> _effect, std::function<float(const std::shared_ptr<WorldState>&)> _cost)
            : name(_name), effect(_effect), cost{ _cost } {}

        void operator()(std::shared_ptr<WorldState>& worldState) {
            effect(worldState);
        }

        [[nodiscard]]float getCost(const std::shared_ptr<WorldState>& worldState) const {
			return cost(worldState);
		}


        [[nodiscard]] std::string getName() const { return name; }
    };


    class Action {
        std::string name{};

        std::vector<Condition> preconditions{};

        std::vector<Effect> effects{};

        Target target{};

    public:
        Action() : name{ "Generic Action" } {};

        Action(std::string _name)
            : name{ _name } {};
        
        Action(std::string _name, const std::vector<Condition> _condition, const std::vector<Effect> _effect, const Target& _target)
            : name{ _name }, preconditions{ _condition }, effects{ _effect }, target{ _target } {
        };
              
        bool canRun(const std::shared_ptr<WorldState>& ws) const;

        bool run(std::shared_ptr<WorldState>& ws) const;

        std::optional<std::shared_ptr<WorldState>> safeRun(const std::shared_ptr<WorldState>& ws) const;
                
        void addPrecondition(const Condition _condition) {
            preconditions.emplace_back(_condition);
        }

        void setPrecondition(std::vector<Condition> _conditions) {
            preconditions = _conditions;
        }

        void addEffect(const Effect _effect) {
            effects.emplace_back(_effect);
           
        }
       
        void setEffect(std::vector<Effect> _effects) {
            effects = _effects;
        }

        [[nodiscard]] float getCost(const std::shared_ptr<WorldState>& ws) const {
            float cost = 0.f; 
            
            for(auto e : effects)
            {
                cost += e.getCost(ws);
            }
                
            return cost;
        }

		[[nodiscard]] std::string getName() const { return name; }

       
        std::string to_string() {
            std::string s = std::string("Action : ") + name +
                std::string("\n\tPréconditions : \n");
            for (const auto& c : preconditions)
               s += std::string( "\t\t" ) +  c.getName() + "\n";
             s += "\tEffets : \n";
            for (const auto& e : effects)
                s+= "\t\t" + e.getName() + "\n";
            return s;
        }

        Target getTarget() const { return target; }
        

	};

    
}

#endif // !GOAPACTION
