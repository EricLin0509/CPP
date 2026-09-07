#include <iostream>
#include <hive>

class Student {
    private:
        std::string name;
        int age;
    public:
        Student(std::string name, int age)
        {
            this->name = name;
            this->age = age;

            std::cout << "Constructor called\n";
        }
        Student(const Student& student)
        {
            this->name = student.name;
            this->age = student.age;

            std::cout << "Copy constructor called\n";
        }
        Student(Student&& student) noexcept
        {
            this->name = std::move(student.name);
            this->age = std::move(student.age);

            std::cout << "Move constructor called\n";
        }
};

int main() {
    std::hive<Student> students;
    Employee stu1("Alice", 25);
    students.insert(emp1); // 调用拷贝构造函数
    students.insert(std::move(emp1));
    students.emplace("Bob", 28); // 调用移动构造函数

    return 0;
}