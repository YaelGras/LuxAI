#ifndef kit_h
#define kit_h
#include <ostream>
#include <string>
#include <iostream>
#include <cmath>
#include <map>
#include <vector>

#include "lux/World/Unit.h"
#include "lux/World/Map.h"
#include "World/Coordinates.h"

using Order = std::string;

namespace kit
{
    enum class ResourceType : char
    {
        wood = 'w',
        coal = 'c',
        uranium = 'u'
    };

    class INPUT_CONSTANTS
    {
    public:
        inline static std::string DONE = "D_DONE";
        inline static std::string RESOURCES = "r";
        inline static std::string RESEARCH_POINTS = "rp";
        inline static std::string UNITS = "u";
        inline static std::string CITY = "c";
        inline static std::string CITY_TILES = "ct";
        inline static std::string ROADS = "ccd";
    };


    static std::string getline()
    {
        // exit if stdin is bad now
        if (!std::cin.good())
            exit(0);

        char str[2048]{}, ch{};
        int i = 0;
        ch = getchar();
        while (ch != '\n')
        {
            str[i] = ch;
            i++;
            ch = std::getchar();
        }

        str[i] = '\0';
        // return the line
        return std::string(str);
    }

    /** return the command to move unit in the given direction */
    static std::string move(const std::string& unit_id, const DIRECTIONS& dir)
    {
        return "m " + unit_id + " " + (char)dir;
    }

    static Order moveUnit(const MovableUnit& unit, DIRECTIONS dir) {
        
        return kit::move(unit.getId(), dir);
    }
    
    // move this
    static DIRECTIONS getDirectionTowardsTile(const MovableUnit& unit, iVec2 target) {

        const iVec2 delta = Coordinates::getStepsToTile(unit.getPos(), target);
        DIRECTIONS dir;
        if (abs(delta.first) < abs(delta.second)) {
            dir = Coordinates::getDirection(unit.getPos(), { 0, sgn<int>(delta.second) });

        }
        else {
            dir = Coordinates::getDirection(unit.getPos(), { sgn<int>(delta.first), 0 });
        }
        return dir;
    }

    static Order moveUnitTowardsTile(const MovableUnit& unit, iVec2 target) {


        DIRECTIONS dir = getDirectionTowardsTile(unit, target);
        return moveUnit(unit, dir);
    }

    static std::vector<std::string> tokenize(std::string s, std::string del = " ")
    {
        std::vector<std::string> strings = std::vector<std::string>();
        std::size_t start = 0;
        std::size_t end = s.find(del);
        while (end != -1)
        {
            strings.push_back(s.substr(start, end - start));
            start = end + del.size();
            end = s.find(del, start);
        }
        strings.push_back(s.substr(start, end - start));
        return strings;
    }


    /** return the command to transfer a resource from a source unit to a destination unit as specified by their ids or the units themselves */
    static std::string transfer(const std::string& src_unit_id, const std::string& dest_unit_id, const ResourceType& resourceType, int amount)
    {
        std::string resourceName;
        switch (resourceType)
        {
        case ResourceType::wood:
            resourceName = "wood";
            break;
        case ResourceType::coal:
            resourceName = "coal";
            break;
        case ResourceType::uranium:
            resourceName = "uranium";
            break;
        }
        return "t " + src_unit_id + " " + dest_unit_id + " " + resourceName + " " + std::to_string(amount);
    }

    /** return the command to build a city right under the worker */
    static std::string buildCity(const std::string& unit_id)
    {
        return "bcity " + unit_id;
    }

    /** return the command to pillage whatever is underneath the worker */
    static std::string pillage(const std::string& unit_id)
    {
        return "p " + unit_id;
    }

    /** returns command to ask this tile to research this turn */
    static std::string research(int x, int y)
    {
        return "r " + std::to_string(x) + " " + std::to_string(y);

    }

    /** returns command to ask this tile to build a worker this turn */
    static std::string buildWorker(int x, int y)
    {
        return "bw " + std::to_string(x) + " " + std::to_string(y);
    }

    /** returns command to ask this tile to build a cart this turn */
    static std::string buildCart(int x, int y)
    {
        return "bc " + std::to_string(x) + " " + std::to_string(y);
    }

    // end a turn
    static void end_turn()
    {
        std::cout << "D_FINISH" << std::endl
            << std::flush;
    }
}

#endif
