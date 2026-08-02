#include <iostream>

void process(int& x)
{
    std::cout << "Left value version: " << x << "\n";
}

void process(int&& x)
{
    std::cout << "Right value version: " << x << "\n";
}

template<typename T>
void wrapper(T&& arg)
{
    process(std::forward<T>(arg));
}

int main() {
    wrapper(10); // 输出: Right value version: 10
    int x = 10;
    wrapper(x); // 输出: Left value version: 10

    return 0;
}