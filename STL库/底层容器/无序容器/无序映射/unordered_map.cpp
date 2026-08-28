#include <iostream>
#include <unordered_map>

struct HashString {
    size_t operator()(const std::string& key) const {
        size_t hash = 0;
        for (int i = 0; i < key.length(); i++)
        {
            hash = (hash << i) ^ key[i];
        }
        return hash;
    }
};

struct CmpByContent {
    bool operator()(const std::string& lhs, const std::string& rhs) const {
        return lhs.length() == rhs.length();
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
};

int main() {
    std::unordered_map<std::string, int, HashString, CmpByContent> umap = {
        {"apple", 1},
        {"banana", 2},
        {"cherry", 3},
        {"date", 4}
    };

    std::cout << "Bucket for key: " << umap.bucket("apple") << "\n";
    std::cout << "Bucket count: " << umap.bucket_count() << "\n";
    std::cout << "Max bucket count of unordered_map: " << std::hex << std::showbase << umap.max_bucket_count() << "\n";
    std::cout << "Load factor: " << umap.load_factor() << "\n";
    std::cout << "Max load factor of unordered_map: " << umap.max_load_factor() << "\n";

    std::cout << "Current max load factor: " << umap.max_load_factor() << "\n";
    umap.max_load_factor(1.5f);
    std::cout << "Current max load factor: " << umap.max_load_factor() << "\n";

    std::unordered_map<int, Employee> employees;
    Employee emp1("John Doe", 1);
    employees.insert({1, emp1});
    employees.insert({2, std::move(emp1)});
    employees.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(3),
        std::forward_as_tuple("Jane Doe", 2)
    );

    return 0;
}
