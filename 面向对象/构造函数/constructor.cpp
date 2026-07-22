#include <iostream>

class Student {
    public:
        std::string name;
        unsigned short age;
        double gpa;

        // 构造函数
        Student(std::string name, unsigned short age, double gpa)
        {
            this->name = name;
            this->age = age;
            this->gpa = gpa;
        }
};

int main() {
    Student student1("Alice", 20, 3.5);
    Student student2("Bob", 22, 3.8);

    std::cout << "Student 1: " << student1.name << ", Age: " << student1.age << ", GPA: " << student1.gpa << "\n";
    std::cout << "Student 2: " << student2.name << ", Age: " << student2.age << ", GPA: " << student2.gpa << "\n";

    return 0;
}