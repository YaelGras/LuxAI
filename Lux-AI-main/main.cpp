#include "lux/agent.hpp"

#include <ostream>
#include <string>
#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include "lux/kit.hpp"

/*
cls && lux-ai-2021 --seed=5654365046015046000 --statefulReplay Unan.exe Unan.exe
cls && lux-ai-2021 --seed=100 --statefulReplay --maxtime=999999 Unan.exe Unan.exe
*/

inline long long nanoTime()
{
    using namespace std::chrono;
    return duration_cast<nanoseconds>(high_resolution_clock::now().time_since_epoch()).count();
}


int main()
{
#ifndef NDEBUG
    std::this_thread::sleep_for(std::chrono::seconds(10));
#endif
    kit::Agent agent = kit::Agent();


    long long start = nanoTime();
    long long total = nanoTime();
    long long sum = 0;


    agent.Initialize();
    std::cerr << "Init time : " << (nanoTime() - start) * 1E-6f << std::endl;

    int turn = 0;
    while (true)
    {
        long long d = nanoTime();
        
        agent.ExtractGameState();

        std::vector<std::string> orders = std::vector<std::string>();
        agent.GetTurnOrders(orders);

        //Send orders to game engine
        for (int i = 0; i < orders.size(); i++)
        {
            if (i != 0)
            {
                std::cout << ",";
            }

            std::cout << orders[i];
        }
        std::cout << std::endl;

        // end turn
        kit::end_turn();
        turn++;
        sum += (nanoTime() - d);
       
        if (turn == 359) {
        std::cerr << "Turn time avg : " << sum * 1E-6f * 1.f / 359.f << "ms" << std::endl;
    }
    }
    std::cerr 
        << "---------------------" << std::endl;
    return 0;
}
