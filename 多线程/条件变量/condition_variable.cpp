#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>

std::mutex mtx;
int fuel = 0;
std::condition_variable cv;

void fuel_filling()
{
    for (int i = 0; i < 5; i++)
    {
        {
            std::lock_guard<std::mutex> lock(mtx);
            fuel += 15;
            std::cout << "加油中... " << fuel << std::endl;
        }
        cv.notify_one(); // 发送条件变量
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

void car()
{
    std::unique_lock<std::mutex> lock(mtx);
    while (fuel < 40)
    {
        std::cout << "没油了，等待中..." << std::endl;
        cv.wait(lock); // 等待条件变量
        // 等价于：
        // lock.unlock();
        // 等待条件变量
        // lock.lock();
    }
    fuel -= 40;
    std::cout << "请立即加油! 现在剩余：" << fuel << std::endl;
}

int main() {
    std::thread t1(fuel_filling);
    std::thread t2(car);

    t1.join();
    t2.join();

    return 0;
}