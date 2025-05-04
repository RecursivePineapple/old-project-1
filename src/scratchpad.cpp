
#pragma target scratchpad

#define PROFILER_PRINT_INTERVAL 1e9

#include <ctime>

#include "Common/linalg.hpp"

#include "Utils/profiler.hpp"

constexpr int NPLAYERS = 3000;

constexpr float radius = 3;

float random_float()
{
    return static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
}

int main() {

    std::vector<linalg::aliases::float3> players(NPLAYERS);

    srand(time(NULL));

    for(size_t i = 0; i < players.size(); i++)
    {
        players[i] = linalg::aliases::float3{
            random_float() * 10 - 5,
            random_float() * 10 - 5,
            random_float() * 10 - 5,
        };
    }
    
    while(true)
    {
        PROFILE_BLOCK

        linalg::aliases::float3 x = {
            random_float() * 10 - 5,
            random_float() * 10 - 5,
            random_float() * 10 - 5,
        };

        int counter = 0;

        for(size_t i = 0; i < players.size(); i++)
        {
            if(linalg::length2((players[i] - x)) <= radius * radius)
            {
                counter++;
            }
        }
    }

    return 0;
}
