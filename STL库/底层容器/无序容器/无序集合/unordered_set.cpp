#include <iostream>
#include <unordered_set>

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
        bool operator==(const Employee& other) const // 使用 `unordered_set` 需要的元素必须实现 `operator==` 方法
        {
            return this->name == other.name && this->age == other.age;
        }

        friend struct std::hash<Employee>;
};

namespace std {
    template <>
    struct hash<Employee> {
        size_t operator()(const Employee& e) const noexcept // 特化 `std::hash` 以支持 `Employee` 类
        {
            size_t h1 = std::hash<std::string>{}(e.name);
            size_t h2 = std::hash<int>{}(e.age);
            return h1 ^ (h2 << 1);
        }
    };
}

int main() {
    std::unordered_set<Employee> employees;
    Employee emp1("John Doe", 1);
    Employee emp2("James May", 2);
    employees.insert(emp1);
    employees.insert(std::move(emp2));
    employees.emplace("Jane Doe", 3);

    auto it = employees.find(Employee("John Doe", 2));
    if (it != employees.end())
    {
        employees.emplace_hint(it, "Bob", 2);
    }
}
