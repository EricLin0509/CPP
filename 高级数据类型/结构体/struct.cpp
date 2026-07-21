#include <iostream>

struct student {
    std::string name;
    unsigned short age;
    double gpa;
};

int main()
{
    student student1 = {"Alice", 20, 3.5};

    std::cout << sizeof(student) << std::endl;

    std::cout << "Name: " << student1.name << std::endl;
    std::cout << "Age: " << student1.age << std::endl;
    std::cout << "GPA: " << student1.gpa << std::endl;

    return 0;
}
