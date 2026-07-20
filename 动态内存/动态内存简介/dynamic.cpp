#include <iostream>

int main()
{
    int *ptr = new int[5]; // 分配一个大小为5的整数数组

    for (int i = 0; i < 5; i++)
    {
        ptr[i] = (i + 1) * 2; // 赋值
    }

    for (int i = 0; i < 5; i++)
    {
        std::cout << ptr[i] << " "; // 使用
    }
    std::cout << "\n";

    delete[] ptr; // 释放内存

    return 0;
}
