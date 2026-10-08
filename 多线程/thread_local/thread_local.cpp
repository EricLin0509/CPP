#include <iostream>
#include <string>
#include <thread>
#include <mutex>

std::mutex g_mutex;

int calculate(void)
{
    std::cout << "The function calculate() is called\n";
    return 42; // 假设计算结果为 42
}

void thread_func(void)
{
    for (int i = 0; i < 10; ++i)
    {
        thread_local int result = calculate();
    }
}

int main(void) {
    std::thread t1(thread_func);
    std::thread t2(thread_func);

    t1.join();
    t2.join();

    return 0;
}