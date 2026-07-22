#include <iostream>

class Human {
    public:
        std::string name;
        int age;

        void eat()
        {
            std::cout << "This person is eating.\n";
        }
        void drink()
        {
            std::cout << "This person is drinking.\n";
        }
        void sleep()
        {
            std::cout << "This person is sleeping.\n";
        }
};

int main() {
    Human person1;
    person1.name = "John";
    person1.age = 25;

    std::cout << "Name: " << person1.name << "\n";
    std::cout << "Age: " << person1.age << "\n";

    person1.eat();
    person1.drink();
    person1.sleep();

    return 0;
}