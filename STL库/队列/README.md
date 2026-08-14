# 队列

相比[栈](../栈/README.md)，队列的元素是先进先出 (FIFO, First In First Out) 的数据结构

在C++中，可以使用 `std::queue` 来实现队列

## 语法

需要引入 `queue` 头文件

```cpp
#include <queue>
```

### 基本声明

```cpp
std::queue<类型> 队列名;
```

### 声明并修改底层实现

```cpp
std::queue<类型, 容器类型> 队列名;
```

- 容器类型：可以是 `std::deque`、`std::list`、`std::vector` 这些底层容器等
- `std::queue` 默认使用 `std::deque` 作为底层容器

## 示例

现在声明一个队列

```cpp
std::queue<int> queue;
```

### 检查队列是否为空

使用 `empty()` 方法检查队列是否为空

```cpp
std::cout << (queue.empty() ? "Queue is empty" : "Queue is not empty") << "\n";
```

### 添加元素

这里有两种方法可以添加元素

- `push()` 方法
- `emplace()` 方法

#### `push()` 方法

```cpp
queue.push(1);
queue.push(2);
queue.push(3);
```

- `push()` 方法会将元素添加到队列的末尾
- `push()` 方法每次只能添加一个元素

#### `emplace()` 方法

相比 `push()` 方法，`emplace()` 方法是直接在队列的末尾构造元素，而不是先创造元素再添加

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
std::queue<Employee> queue;
Employee e1("Alice", 1);

queue.push(e1); // 复制构造
queue.push(std::move(e1)); // 移动构造
queue.emplace("Bob", 2); // 直接构造
```

此时应该会输出

```bash
Constructor called # 这个是 e1 的构造函数
Copy constructor called
Move constructor called
Constructor called
```

这里效率最高的是 `emplace()` 方法，因为它直接在队列中构造元素，避免了复制或移动操作

- `emplace()` 方法里面使用的是可变参数列表，可以方便地构造对象
    - 但**不能**用于一次添加多个元素
- 有关移动语义，请查看[右值引用](../../现代C++特性/右值引用/README.md)

### 移除元素

使用 `pop()` 方法移除队列的头部元素

```cpp
queue.pop();
```

### 获取队列的头部元素

使用 `front()` 方法获取队列的头部元素

```cpp
std::cout << "Front element: " << queue.front() << "\n";
```

### 获取队列的尾部元素

使用 `back()` 方法获取队列的尾部元素

```cpp
std::cout << "Back element: " << queue.back() << "\n";
```

### 获取队列的大小

使用 `size()` 方法获取队列的大小

```cpp
std::cout << "Queue size: " << queue.size() << "\n";
```

### 交换队列

使用 `swap()` 方法交换两个队列的内容

```cpp
std::queue<int> queue2;
queue2.push(4);
queue2.push(5);

queue.swap(queue2);
```

### 比较队列

由于 `std::queue` 实现了 `operator==` 和 `operator!=`，所以可以直接比较两个队列的内容是否相同

```cpp
std::queue<int> queue3;
queue3.push(1);
queue3.push(2);
queue3.push(3);

std::queue<int> queue4;
queue4.push(4);
queue4.push(5);
queue4.push(6);

std::cout << (queue3 == queue4 ? "Queues are equal" : "Queues are not equal") << "\n";
```

### 遍历队列

跟栈一样，`std::queue` 没有实现 `begin()` 和 `end()` 方法，所以不能直接使用 `for` 循环遍历

但可以依次取出队列中的元素并进行处理，直到队列为空

```cpp
while (!queue.empty())
{
    std::cout << "Element: " << queue.front() << "\n";
    queue.pop();
}
```


