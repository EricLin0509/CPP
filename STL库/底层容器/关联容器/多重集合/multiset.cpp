#include <iostream>
#include <set>

class Employee {
    private:
        std::string name;
        int age;
    public:
        Employee(const char *name, int age)
        {
            this->name = name;
            this->age = age;

            std::cout << "Constructor called\n";
        }
        Employee(const Employee& other)
        {
            this->name = other.name;
            this->age = other.age;

            std::cout << "Copy constructor called\n";
        }
        Employee(Employee&& other)
        {
            this->name = std::move(other.name);
            this->age = std::move(other.age);

            std::cout << "Move constructor called\n";
        }

        bool operator<(const Employee& other) const // set需要重载 `<` 运算符
        {
            return age < other.age;
        }
};

int main() {
    std::multiset<Employee> employees;
    Employee alice("Alice", 30);
    employees.insert(std::move(alice));
    employees.emplace("Alice", 30);

    auto it = employees.find(Employee("Alice", 30));
    if (it != employees.end())
    {
        employees.emplace_hint(it, "James", 25);
    }

    Employee bob("Bob", 25);
    for (int i = 0; i < 5; i++)
    {
        employees.insert(bob);
    }

    std::cout << "Bob's count: " << employees.count(bob) << "\n";

    return 0;
}
