#include <iostream>

int gloNum = 10; // 全局变量

void myFunc();

int main()
{
    int myNum = 10; // 局部变量

    std::cout << gloNum << std::endl;

    myFunc();

    return 0;
}

void myFunc()
{
    int myNum = 20; // 局部变量

    std::cout << myNum << std::endl;

    std::cout << gloNum << std::endl;
}
