#include <iostream>

class Animal {
    public:
        bool isAlive = true;

        void eat() {
            std::cout << "This animal is eating" << "\n";
        }
};

class Dog : public Animal {
    public:
        void bark()
        {
            std::cout << "The dog goes woof!" << "\n";
        }
};

class Cat : public Animal {
    public:
        void meow()
        {
            std::cout << "The cat goes meow!" << "\n";
        }
};

int main() {
    Dog dog;

    std::cout << (dog.isAlive ? "true" : "false") << "\n"; // 输出：true
    dog.eat(); // 输出：This animal is eating
    dog.bark(); // 输出：The dog goes woof!

    Cat cat;
    std::cout << (cat.isAlive ? "true" : "false") << "\n"; // 输出：true
    cat.eat(); // 输出：This animal is eating
    cat.meow(); // 输出：The cat goes meow!

    return 0;
}