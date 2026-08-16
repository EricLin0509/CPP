#include <iostream>
#include <queue>

class Employee {
    private:
        std::string name;
        int id;
    public:
        Employee(std::string name, int id) // 直接构造
        {
            this->name = name;
            this->id = id;

            std::cout << "Constructor called\n";
        }
        Employee(const Employee& e) // 复制构造
        {
            this->name = e.name;
            this->id = e.id;

            std::cout << "Copy constructor called\n";
        }
        Employee(Employee&& e) noexcept // 移动构造 (移动语义)
        {
            this->name = std::move(e.name);
            this->id = std::move(e.id);

            std::cout << "Move constructor called\n";
        }
        friend std::ostream& operator<<(std::ostream& os, const Employee& e)
        {
            os << "Employee(" << e.name << ", " << e.id << ")";
            return os;
        }

};

int main() {
    std::queue<Employee> queue;
    Employee e1("Alice", 1);

    queue.push(e1); // 复制构造
    queue.push(std::move(e1)); // 移动构造
    queue.emplace("Bob", 2); // 直接构造
    
    std::cout << "Queue size: " << queue.size() << "\n";
    std::cout << "Front element: " << queue.front() << "\n";
    std::cout << "Back element: " << queue.back() << "\n";

    while (!queue.empty())
    {
        std::cout << "Element: " << queue.front() << "\n";
        queue.pop();
    }

    return 0;
}