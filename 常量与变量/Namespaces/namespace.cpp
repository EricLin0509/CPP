#include <iostream>

namespace globalNamespace {
    int a = 20;
}

namespace usingTest {
    int b = 30;
}

int main()
{
    int a = 10;
    std::cout << "a = " << a << std::endl;
    std::cout << "globalNamespace::a = " << globalNamespace::a << std::endl;

    using usingTest::b;
    std::cout << "usingTest::b = " << b << std::endl;

    return 0;
}