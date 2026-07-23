#include <iostream>

class Person {
    protected:
        std::string name = "John";
        int age = 20;
};

class Student : public Person {
    public:
        void display()
        {
            std::cout << "Name: " << name << "\n";
            std::cout << "Age: " << age << "\n";
        }
};

int main() {
    Student stu;
    stu.display(); // Output: Name: John Age: 20

    return 0;
}