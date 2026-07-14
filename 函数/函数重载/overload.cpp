#include <iostream>

int add(int a, int b)
{
    return a + b;
}

int add(int a, int b, int c)
{
    return a + b + c;
}

int add(int a, int b, int c, int d)
{
    return a + b + c + d;
}

int main()
{
    std::cout << add(1, 2) << "\n";
    std::cout << add(1, 2, 3) << "\n";
    std::cout << add(1, 2, 3, 4) << "\n";

    return 0;
}
