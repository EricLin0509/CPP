#include <iostream>

int main()
{
    std::string arr[100];

    // 使用 fill 函数填充数组
    std::fill(arr, arr + 100, "Hello");

    // 输出数组内容
    for (std::string str : arr)
    {
        std::cout << str << "\n";
    }

    return 0;
}