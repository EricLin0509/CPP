# 栈

栈 (Stack) 是一种数据结构，它遵循后进先出 (LIFO, Last In First Out) 的原则

栈通常用于存储和管理一系列元素，例如函数调用栈、表达式求值栈等

在C++中，栈通常使用 STL 库中的 `std::stack` 类来实现

## 语法

需要引入 `stack` 头文件

```cpp
#include <stack>
```

### 基本声明

```cpp
std::stack<类型> 栈名;
```

### 声明并修改底层实现

由于栈的定义如下

```cpp
template<class T, class Container = std::deque<T>>
class stack;
```

- `T` 是栈中元素的类型 (上面的 `类型` 指的就是这个参数)
- `Container` 是栈中元素的容器类型，默认是 `std::deque<T>`，即双端队列
    - 由于 `std::stack` 是**容器适配器**，所以需要依赖这些底层容器
    - 也可以是 `std::vector<T>`、`std::list<T>` 等其他底层容器类型

所以可以使用 STL 库中的底层容器类型来声明并初始化栈

```cpp
std::vector<int> vec = {1, 2, 3};
std::stack<int, std::vector<int>> stack(vec);
```

- **注意**: 这里在栈上操作不会影响原容器的内容，因为构造函数是将原容器的内容复制到栈中
    - 只是影响了栈的底层实现

## 示例

现在创建一个栈，存储整数元素

```cpp
std::stack<int> stack;
```

### 检查是否为空

使用 `empty()` 方法检查栈是否为空

```cpp
std::cout << "Is stack empty: " << (stack.empty() ? "Yes" : "No") << std::endl;
```

### 入栈

这里有两种方法可以将元素压入栈中

- `push()`
- `emplace()`

#### `push()` 方法

```cpp
stack.push(1);
stack.push(2);
stack.push(3);
```

- `push()` 方法一次只能压入一个元素

#### `emplace()` 方法

相比 `push()` 方法，`emplace()` 方法是直接在栈中构造元素，而不是先构造元素再复制到栈中

这在配合对象构造函数时非常好用

```cpp
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

};
```

```cpp
std::stack<Employee> stack;
Employee e1("Bob", 2);

stack.push(e1);
stack.push(std::move(e1));
stack.emplace("Alice", 1);
```

查看输出

```bash
Constructor called # 这个是 e1 的构造函数
Copy constructor called
Move constructor called
Constructor called
```

这里效率最高的是 `emplace()` 方法，因为它直接在栈中构造元素，避免了复制或移动操作

- `emplace()` 方法里面使用的是可变参数列表，可以方便地构造对象
    - 但**不能**用于一次压入多个元素
- 有关移动语义，请查看[右值引用](../../现代C++特性/右值引用/README.md)

### 出栈

使用 `pop()` 方法将栈顶元素弹出栈中

```cpp
stack.pop();
```

- `pop()` 方法一次只能弹出一个元素

### 获取栈顶元素

使用 `top()` 方法获取栈顶元素

```cpp
std::cout << "Top element: " << stack.top() << std::endl;
```

### 获取栈中元素个数

使用 `size()` 方法获取栈中元素个数

```cpp
std::cout << "Number of elements: " << stack.size() << std::endl;
```

### 交换栈

使用 `swap()` 方法交换两个栈的内容

```cpp
std::vector<int> vec2 = {4, 5, 6};
std::stack<int, std::vector<int>> stack2(vec2);
stack.swap(stack2);
```

### 比较栈

由于 `std::stack` 实现了 `operator==` 和 `operator!=`，所以可以直接比较两个栈的内容是否相同

```cpp
std::stack<int> stack3;
stack3.push(1);
stack3.push(2);
stack3.push(3);

std::stack<int> stack4;
stack4.push(4);
stack4.push(5);
stack4.push(6);

std::cout << "stack3 == stack4: " << (stack3 == stack4 ? "Yes" : "No") << std::endl;
```

### 遍历栈

由于 `std::stack` 没有实现 `begin()` 和 `end()` 方法，所以不能直接使用 `for` 循环遍历栈中的元素

但可以依次弹出栈顶元素并打印出来，直到栈为空

```cpp
while (!stack.empty())
{
    std::cout << stack.top() << " ";
    stack.pop();
}
std::cout << "\n";
```

