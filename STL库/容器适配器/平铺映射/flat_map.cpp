#include <iostream>
#include <flat_map>

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

        Employee& operator=(const Employee& other)
        {
            this->name = other.name;
            this->age = other.age;

            return *this;
        }
        friend std::ostream& operator<<(std::ostream& os, const Employee& emp)
        {
            os << "{Name: " << emp.name << ", Age: " << emp.age << "}";
            return os;
        }
};

int main() {
    std::flat_map<int, Employee> employees;
    Employee emp1("John Doe", 1);
    employees.insert({1, emp1});
    employees.insert({2, std::move(emp1)});
    employees.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(3),
        std::forward_as_tuple("Jane Doe", 2)
    );
    employees.try_emplace(4, "Alice", 3);

    for (const auto& pair : employees)
    {
        std::cout << pair.first << ": " << pair.second << "\n";
    }

    auto range = employees.equal_range(2);
    std::cout << "Lower bound: " << range.first->first << " " << range.first->second << "\n";
    std::cout << "Upper bound: " << range.second->first << " " << range.second->second << "\n";

    return 0;
}