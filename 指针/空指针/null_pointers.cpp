#include <iostream>

int main()
{
    int *null = nullptr; // 使用 nullptr 初始化空指针

    if (null != nullptr)
    {
        std::cout << "pointer is assigned" << "\n";
    }
    else 
    {
        std::cout << "pointer is not assigned" << "\n";
    }

    int *ptr = nullptr; // 使用 nullptr 初始化空指针
    int a = 10;
    ptr = &a; // 将 ptr 指向变量 a

    if (ptr != nullptr)
    {
        std::cout << "pointer is assigned" << "\n";
    }
    else 
    {
        std::cout << "pointer is not assigned" << "\n";
    }

    return 0;
}