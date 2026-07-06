#include <iostream>

int main() {
    int a = 5;

    a = a + 3; // 算术运算符：加法
    std::cout << "a = " << a << std::endl;

    a++; // 算术运算符：自增
    std::cout << "a = " << a << std::endl;

    a = a - 2; // 算术运算符：减法
    std::cout << "a = " << a << std::endl;

    a--; // 算术运算符：自减
    std::cout << "a = " << a << std::endl;

    a = a * 4; // 算术运算符：乘法
    std::cout << "a = " << a << std::endl;

    a = a / 2; // 算术运算符：除法
    std::cout << "a = " << a << std::endl;

    a = a % 3; // 算术运算符：取模
    std::cout << "a = " << a << std::endl;

    return 0;
}