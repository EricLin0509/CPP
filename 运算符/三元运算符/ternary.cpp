#include <iostream>

int main()
{
    int num = 0;
    std::cout << "请输入一个数字: ";
    std::cin >> num;

    std::cout << ((num & 1) == 0 ? "偶数" : "奇数") << std::endl;

    return 0;
}