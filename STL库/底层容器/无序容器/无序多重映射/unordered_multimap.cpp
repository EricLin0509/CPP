#include <iostream>
#include <unordered_map>

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
    std::unordered_multimap<int, Employee> employees;
    Employee emp1("John Doe", 1);
    employees.insert({1, emp1});
    employees.insert({2, std::move(emp1)});
    employees.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(3),
        std::forward_as_tuple("Jane Doe", 2)
    );

    auto it = employees.find(3);
    if (it != employees.end())
    {
        employees.emplace_hint(it,
            std::piecewise_construct,
            std::forward_as_tuple(4),
            std::forward_as_tuple("Bob", 4)
        );
    }

    Employee emp2("Alice", 3);
    employees.insert({5, emp2});
    employees.insert({5, emp2});
    employees.insert({5, emp2});

    std::cout << "The count of the key 5 is: " << employees.count(5) << "\n";
}
