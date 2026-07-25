#include <iostream>

class Person {
    private:
        std::string name;
    public:
        Person() {}

        Person(std::string name)
        {
            this->name = name;
        }

        // 插入符重载
        friend std::ostream& operator<<(std::ostream& os, Person& p)
        {
            os << "Welcome back " << p.name << "\n";
            return os;
        }

        friend std::istream& operator>>(std::istream& is, Person& p);
};

// 输入符重载
std::istream& operator>>(std::istream& is, Person& p)
{
    std::cout << "Enter your name: ";
    is >> p.name;
    return is;
}

int main() {
    Person p;
    std::cin >> p;
    std::cout << p << "\n";

    return 0;
}