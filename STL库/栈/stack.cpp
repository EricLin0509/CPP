#include <iostream>
#include <vector>
#include <stack>

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
        Employee(Employee&& e) // 移动构造 (移动语义)
        {
            this->name = std::move(e.name);
            this->id = std::move(e.id);

            std::cout << "Move constructor called\n";
        }

        void print()
        {
            std::cout << "Name: " << name << "\nID: " << id << "\n";
        }

};

int main() {
    std::stack<Employee> stack;
    Employee e1("John", 1);
    stack.push(e1); // 这里需要一次昂贵的内存拷贝
    stack.push(std::move(e1)); // 这里虽说有右值移动提高效率，但依旧需要进行一次所有权移动
    stack.emplace("Eric", 2); // 这里emplace会直接在栈上构造对象，效率更高

    while (!stack.empty())
    {
        Employee e = stack.top();
        e.print();
        stack.pop();
    }

    return 0;
}