#include <iostream>
#include <thread>

void thread_func()
{
    std::cout << "Hello from thread " << std::this_thread::get_id() << "\n";
}

int main() {
    std::cout << "Thread ID: " << std::this_thread::get_id() << "\n";
    std::cout << "CPU cores: " << std::thread::hardware_concurrency() << "\n";

    std::thread t1(thread_func);
    std::cout << "Is t1 joinable: " << (t1.joinable() ? "Yes" : "No") << "\n";

    if (t1.joinable())
        t1.join();

    std::cout << "Is t1 joinable: " << (t1.joinable() ? "Yes" : "No") << "\n";

    if (std::thread::id() == t1.get_id())
        std::cout << "Invalid thread ID\n";

    return 0;
}