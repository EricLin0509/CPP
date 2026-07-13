#include <iostream>

int main()
{
    int n = 0;
    std::cout << "请输入一个正整数: ";
    std::cin >> n;

    if (n <= 0)
    {
        std::cout << "无效输入。请输入一个正整数。" << std::endl;
        return 1;
    }

    int spaces = n - 1;
    for (int i = 1; i <= n; i++)
    {
        for (int j = 0; j < spaces; j++)
        {
            std::cout << " "; // 输出空格
        }
        

        for (int k = 0; k < (2 * i - 1); k++)
        {
            std::cout << "*"; // 输出星号
        }

        spaces--; // 减少空格数
        std::cout << "\n";
    }

    return 0;
}