#include <iostream>

int main()
{
    int num = 10;

    if (num >= 0 && num <= 100)
    {
        // num 在 0 到 100 之内
        std::cout << "num 在 0 到 100 之内" << std::endl;
    }

    if (num < 0 || num > 100)
    {
        // num 不在 0 到 100 之内
        std::cout << "num 不在 0 到 100 之内" << std::endl;
    }

    if (!(num < 0 || num > 100))
    {
        // num 不在 0 到 100 之内
        std::cout << "num 在 0 到 100 之内" << std::endl;
    }

    return 0;
}