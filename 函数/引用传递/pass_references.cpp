#include <iostream>

void swap(int &a, int &b)
{
    a ^= b;
    b ^= a;
    a ^= b;
}

int main()
{
    int a = 10;
    int b = 20;

    swap(a, b);

    std::cout << "a: " << a << std::endl;
    std::cout << "b: " << b << std::endl;

    return 0;
}