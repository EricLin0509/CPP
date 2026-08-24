#include <iostream>
#include <map>

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
};

int main() {
    std::multimap<int, Employee> employees;
    Employee emp("Alice", 30);
    employees.insert({1, emp});
    employees.insert({1, Employee("Bob", 25)});
    employees.insert({1, Employee("Charlie", 35)});
    employees.insert({2, std::move(emp)});
    employees.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(3),
        std::forward_as_tuple("Jane Doe", 2)
    );
    
    auto it = employees.find(2);
    if (it != employees.end())
    {
        employees.emplace_hint(it, std::piecewise_construct,
                            std::forward_as_tuple(4),
                            std::forward_as_tuple("James May", 3));
    }

    std::cout << "Count of key 1: " << employees.count(1) << "\n";
}