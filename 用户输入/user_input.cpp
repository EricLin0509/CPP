#include <iostream>

int main()
{
    int age = 0;
    std::cin >> age;

    std::string name = "";
    std::getline(std::cin >> std::ws, name);

    std::cout << "Your name is " << name << std::endl;
    std::cout << "Your age is " << age << std::endl;
}