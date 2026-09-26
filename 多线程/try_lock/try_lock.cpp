#include <iostream>
#include <mutex>
#include <thread>
#include <chrono>
#include <random>

static constexpr int STOVE_COUNT = 4;
static constexpr int THREAD_COUNT = 10;

static std::mutex stove_mutexes[STOVE_COUNT];
static int stove_fuel[STOVE_COUNT] = {100, 100, 100, 100};

void routine()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(1, 30);

    for (int i = 0; i < STOVE_COUNT; i++)
    {
        if (stove_mutexes[i].try_lock())
        {
            int needed_fuel = dist(gen);

            if (stove_fuel[i] - needed_fuel < 0)
            {
                std::cout << "炉子 " << i << " 燃料不足\n";
            }
            else
            {
                stove_fuel[i] -= needed_fuel;
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
                std::cout << "炉子 " << i << " 消耗 " << needed_fuel
                          << "，剩余 " << stove_fuel[i] << "\n";
            }

            stove_mutexes[i].unlock();
            return;
        }
        else
        {
            if (i == STOVE_COUNT - 1)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(300));
                i = -1;
            }
        }
    }
}

int main()
{
    std::thread threads[THREAD_COUNT];

    for (auto& t : threads)
    {
        t = std::thread(routine);
    }

    for (auto& t : threads)
    {
        t.join();
    }

    return 0;
}
