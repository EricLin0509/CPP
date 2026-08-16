#include <iostream>
#include <deque>

class Student {
    private:
        std::string name;
    public:
        Student(std::string name) // 直接构造
        {
            this->name = name;

            std::cout << "Constructor called\n";
        }
        Student(const Student &student) // 拷贝构造
        {
            this->name = student.name;

            std::cout << "Copy constructor called\n";
        }
        Student(Student &&student) noexcept // 移动构造
        {
            this->name = std::move(student.name);

            std::cout << "Move constructor called\n";
        }

        Student &operator=(Student &&student) noexcept // 移动赋值
        {
            if (this != &student)
                this->name = std::move(student.name);

            std::cout << "Move assignment called\n";

            return *this;
        }

        friend std::ostream &operator<<(std::ostream &os, const Student &student)
        {
            std::cout << "Student: " << student.name << "\n";
            return os;
        }
};

int main() {
    std::deque<Student> students;
    Student s1("Alice");

    students.insert(students.begin(), s1); // 使用拷贝构造
    students.insert(students.begin(), std::move(s1)); // 使用移动构造
    students.emplace(students.begin(), "Bob"); // 使用直接构造

    students.emplace_front("Charlie");
    students.emplace_back("David");

    try
    {
        std::cout << students.at(0);
    }
    catch (const std::out_of_range& e)
    {
        std::cout << "Index out of range\n";
    }

    for (const auto &student : students)
    {
        std::cout << student;
    }
    std::cout << "\n";

    for (auto it = students.begin(); it != students.end(); it++)
    {
        std::cout << *it;
    }

    return 0;
}