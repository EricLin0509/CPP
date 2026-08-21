#include <iostream>
#include <map>

struct CmpByLength {
    bool operator()(const std::string& x, const std::string& y) const {
        return x.length() < y.length();
    }
};

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

        friend std::ostream& operator<<(std::ostream& os, const Employee& emp)
        {
            os << "Employee(" << emp.name << ", " << emp.age << ")";
            return os;
        }
};

int main() {
    std::map<std::string, int, CmpByLength> map;
    map.insert({"key", 1});
    std::cout << "Size: " << map.size() << "\n";

    try
    {
        std::cout << map.at("key") << "\n";
    }
    catch (const std::out_of_range& e)
    {
        std::cout << "Key not found: " << e.what() << "\n";
    }

    auto result = map.insert({"key", 2}); // 这里会插入失败，因为key已经存在
    if (result.second)
    {
        std::cout << "Key inserted successfully.\n";
    }
    else
    {
        std::cout << "Key already exists.\n";
    }

    map.insert_or_assign("key", 100); // 这里会插入成功，因为它会覆盖key的值

    auto it = map.find("key");
    if (it != map.end())
    {
        std::cout << "Key found: " << it->first << " " << it->second << "\n";
    }
    else
    {
        std::cout << "Key not found\n";
    }

    map.erase("key");

    if (map.contains("key"))
    {
        std::cout << "Key found\n";
    }
    else
    {
        std::cout << "Key not found\n";
    }

    std::map<int, Employee> emp_map;
    Employee emp1("John Doe", 1);
    emp_map.insert({1, emp1});
    emp_map.insert({2, std::move(emp1)});
    emp_map.try_emplace(3, "Jane Doe", 2);

    for (const auto& pair : emp_map)
    {
        std::cout << "{" << pair.first << ", " << pair.second << "}\n";
    }

    std::map<int, std::string> numbers = {
        {0, "zero"},
        {1, "one"},
        {2, "two"},
        {3, "three"},
        {4, "four"}
    };

    auto num1 = numbers.lower_bound(2);
    std::cout << "Lower bound: " << num1->first << " " << num1->second << "\n";
    auto num2 = numbers.upper_bound(2);
    std::cout << "Upper bound: " << num2->first << " " << num2->second << "\n";
    auto range = numbers.equal_range(2);
    std::cout << "Lower bound: " << range.first->first << " " << range.first->second << "\n";
    std::cout << "Upper bound: " << range.second->first << " " << range.second->second << "\n";

    return 0;
}