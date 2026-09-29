#include <iostream>
#include <thread>
#include <atomic>

std::atomic<bool> flag = false;

void wait()
{
    std::cout << "Wait for flag to be true..." << "\n";
    flag.wait(false);
    std::cout << "Flag is true, continue..." << "\n";
}

void notify()
{
    std::this_thread::sleep_for(std::chrono::seconds(2));
    flag.store(true);
    flag.notify_all();
}

int main() {
    std::thread t1(wait);
    std::thread t2(notify);
    t1.join();
    t2.join();

    std::cout << "Flag is " << (flag.load() ? "true" : "false") << "\n";
    return 0;
}