#include <iostream>
#include <thread>
#include <mutex>

static size_t count = 0;

void increment_count()
{
    for (size_t i = 0; i < 1000000; i++)
    {
        count_mutex.lock();
        count++;
        count_mutex.unlock();
    }
}

int main(void) {
    std::thread threads[2];

    for (auto& thread : threads)
    {
        thread = std::thread(increment_count);
    }

    for (auto& thread : threads)
    {
        thread.join();
    }

    std::cout << "count: " << count << "\n";

    return 0;
}