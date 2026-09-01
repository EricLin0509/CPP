#include <iostream>

template <typename T>
T max(T a, T b)
{
    return a > b ? a : b;
}

template <typename T1, typename T2>
size_t max_size(T1 a, T2 b)
{
    return sizeof(a) > sizeof(b) ? sizeof(a) : sizeof(b);
}

int main() {
    int a = 10;
    int b = 20;
    int result1 = max(a, b); // result = 20
    std::cout << "Max of " << a << " and " << b << " is " << result1 << "\n";

    double c = 10.5;
    double d = 20.5;
    double result2 = max(c, d); // result = 20.5
    std::cout << "Max of " << c << " and " << d << " is " << result2 << "\n";

    int e = 10;
    double f = 20.5;
    size_t result3 = max_size(e, f); // result = 20.5
    std::cout << "The size of " << e << " and " << f << " is " << result3 << "\n";

    return 0;
}