#include <iostream>

void print_sum(int a, int b)
{
    std::cout << a + b << "\n";
}

int main()
{
    print_sum(2, 3);
    print_sum(4, 5);
    print_sum(6, 7);

    return 0;
}