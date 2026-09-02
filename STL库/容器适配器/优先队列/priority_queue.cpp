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
        Employee& operator=(Employee&& e) noexcept
        {
            if (this != &e)
            {
                this->name = std::move(e.name);
                this->id = std::move(e.id);
            }

            return *this;
        }
        bool operator<(const Employee& other) const
        {
            return this->id < other.id; // 比较id
        }
};

struct ReverseOrder {
    bool operator()(const int& a, const int& b)
    {
        return a > b; // 降序比较,创建最小堆(小的元素优先级高)
    }
};

int main() {
    std::priority_queue<int, std::vector<int>, ReverseOrder> pq;
    pq.push(1);
    pq.push(2);
    pq.push(3);

    auto reverse_order = [](const int& a, const int& b) { return a > b; };
    std::priority_queue<int, std::vector<int>, decltype(reverse_order)> test_queue(reverse_order);

    while (!pq.empty())
    {
        std::cout << pq.top() << " ";
        pq.pop();
    }
    std::cout << "\n";

    std::priority_queue<Employee> queue;
    Employee e1("Alice", 1);

    queue.push(e1); // 复制构造
    queue.push(std::move(e1)); // 移动构造
    queue.emplace("Bob", 2); // 直接构造

    return 0;
}
