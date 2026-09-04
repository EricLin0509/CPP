#include <iostream>
#include <flat_set>

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

        bool operator<(const Employee& other) const
        {
            return this->age < other.age;
        }

        friend std::ostream& operator<<(std::ostream& os, const Employee& emp)
        {
            os << "{ Employee: " << emp.name << ", Age: " << emp.age << " }";
            return os;
        }
};

int main() {
    std::flat_set<Employee> employees;
    Employee emp1("John Doe", 1);
    Employee emp2("James May", 2);
    employees.insert(emp1);
    employees.insert(std::move(emp2));
    employees.emplace("Jane Doe", 2);

    auto it = employees.find(Employee("John Doe", 2));
    if (it != employees.end())
    {
        std::cout << "Element found: " << *it << "\n";
    }
    else
    {
        std::cout << "Element not found\n";
    }

    std::flat_set<int> numbers = {0, 1, 2, 3, 4, 5};

    auto lower = numbers.lower_bound(2);
    std::cout << "Lower bound: " << *lower << "\n";

    auto range = numbers.equal_range(2);
    std::cout << "Lower bound: " << *range.first << "\n";
    std::cout << "Upper bound: " << *range.second << "\n";

    return 0;
}