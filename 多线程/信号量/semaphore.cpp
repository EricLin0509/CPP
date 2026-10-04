#include <iostream>
#include <thread>
#include <semaphore>

static std::counting_semaphore<2> semaphore(2);

void task(int id)
{
    semaphore.acquire();
    std::this_thread::sleep_for(std::chrono::seconds(1)); // 休眠1秒，模拟任务执行时间
    std::cout << "Hello from thread " << id << "\n";
    semaphore.release();
}

int main() {
    std::thread threads[4];
    
    for (int i = 0; i < 4; i++)
    {
        threads[i] = std::thread(task, i);
    }

    for (int i = 0; i < 4; i++)
    {
        threads[i].join();
    }
}
