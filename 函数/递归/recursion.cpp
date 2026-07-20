#include <iostream>

int fibonacci(int n)
{
    if(n <= 2)
    {
        return 1;
    }
    else
    {
        return fibonacci(n - 1) + fibonacci(n - 2);
    }
}

int main()
{
    int n = 0;

    std::cout << "请输入一个正整数：";
    std::cin >> n;

    if (n <= 0)
    {
        std::cout << "请输入一个正整数！" << std::endl;
        return 1;
    }

    std::cout << "斐波那契数列的第" << n << "项是：" << fibonacci(n) << "\n"; // 调用函数

    return 0;
}


