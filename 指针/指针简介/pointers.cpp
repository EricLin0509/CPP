#include <iostream>

int main()
{
    int a = 10;
    int *p = &a;

    std::cout << "The size of pointer is: " << sizeof(p) << "\n";

    int b = *p; // b = 10

    std::cout << "The value of a is: " << a << "\n";
    std::cout << "The value of b is: " << b << "\n";
}