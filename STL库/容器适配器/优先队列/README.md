# 优先队列

优先队列 (`std::priority_queue`) 是 C++ STL 库中的容器适配器

## 与 `std::queue` 的区别

| 特征 | `std::queue` | `std::priority_queue` |
| :--: | :----------: | :-------------------: |
| 排序策略 | FIFO（先进先出） | 优先移除优先级最高 (或最低) 的元素 |
| 内存结构 | 一段连续内存 | 通常使用二叉堆 (用数组表示的二叉树) 来实现 |
| 插入/删除 | O(1) | O(log n) |

- 有关二叉堆，请参考[heap.cpp](heap.cpp) (此处实现的是最小堆)
    - 如果要实现最大堆，只需要改两处
        - `heap[current] < heap[parent]` -> `heap[current] > heap[parent]`
        - `heap[current] > heap[min_child_index]` -> `heap[current] < heap[max_child_index]` (同时需要修改查找子节点的逻辑)

队列的适用场景：
- 需要保证顺序和公平性的场景
    - 打印缓冲处理
    - 基于到达时间的任务调度

优先队列的适用场景：
- 当紧急程度或重要性比到达顺序更重要时
    - Dijkstra 最短路径算法
    - 任务调度

## 语法

### 基本声明

```cpp
std::priority_queue<类型> 队列名;
```

### 自定义底层容器

```cpp
std::priority_queue<类型, 容器类型> 队列名;
```

- 容器类型：需要支持随机访问的底层容器如 `std::vector`、`std::deque`，`std::list` 不支持
    - 默认情况下，`std::priority_queue` 使用 `std::vector` 作为底层容器


### 自定义比较函数

```cpp
std::priority_queue<类型, 容器类型, 比较函数> 队列名;
```

需要的比较函数原型如下

```cpp
// 方式1: 自定义比较函数对象(推荐)
template <class T>
struct Compare {
    bool operator()(const T& a, const T& b) const;
};

// 方式2: 类成员运算符
class YourClass {
public:
    bool operator<(const YourClass& other) const;
};
```

## 示例

现在创建一个 `priority_queue`

```cpp
std::priority_queue<int> queue;
```

### 自定义比较函数

STL 要求比较器是一个可调用对象类型(函数对象、lambda等)，所以需要使用结构体包装 `operator()` 或使用 lambda 表达式

```cpp
struct ReverseOrder {
    bool operator()(const int& a, const int& b)
    {
        return a > b; // 降序比较,创建最小堆(小的元素优先级高)
    }
};
```

```cpp
std::priority_queue<int, std::vector<int>, ReverseOrder> queue;
```

#### 使用 lambda 表达式

使用 lambda 表达式也可以实现

```cpp
auto reverse_order = [](const int& a, const int& b) { return a > b; };
std::priority_queue<int, std::vector<int>, decltype(reverse_order)> queue(reverse_order);
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
```

```cpp
std::priority_queue<Employee> queue;
Employee e1("Alice", 1);

queue.push(e1); // 复制构造
queue.push(std::move(e1)); // 移动构造
queue.emplace("Bob", 2); // 直接构造
```

此时应该会输出

```bash
Constructor called # 这个是 e1 的构造函数
Copy constructor called # 这个是直接使用 `push` 方法
Move constructor called
Move constructor called
Move constructor called # 这个是 `push` + 右值引用
Move constructor called
Move constructor called
Move constructor called
Constructor called # 这个是使用 `emplace` 方法
Move constructor called
Move constructor called
Move constructor called
Move constructor called
```

- 由于 `priority_queue` 要求类型必须可比较，所以需要在类中实现 `operator<` 方法
    - 比较函数：对应 `operator<` 方法
- 这里出现多次 `Move constructor called`，主要原因有两个：
  1. `priority_queue` 底层使用 `std::vector`，容量不足时会扩容并移动所有现有元素
  2. 插入元素后需要调整堆结构(`push_heap`)，这会通过移动操作重新排列元素位置

相对而言，`emplace()` 方法的效率较高，因为它直接在容器内构造元素，省去了外部对象的创建开销，但堆结构调整和可能的扩容仍会触发内部移动

- `emplace()` 方法里面使用的是可变参数列表，可以方便地构造对象
    - 但**不能**用于一次添加多个元素
- 有关移动语义，请查看[右值引用](../../../现代C++特性/右值引用/README.md)

### 移除元素

使用 `pop()` 方法移除队列的头部元素

```cpp
queue.pop();
```

### 获取队列的头部元素

使用 `top()` 方法获取队列的头部元素

```cpp
std::cout << "Top element: " << queue.top() << "\n";
```

### 获取队列的大小

使用 `size()` 方法获取队列的大小

```cpp
std::cout << "Queue size: " << queue.size() << "\n";
```

### 交换队列

使用 `swap()` 方法交换两个队列的内容

```cpp
std::priority_queue<int> queue2;
queue2.push(4);
queue2.push(5);

queue.swap(queue2);
```

### 遍历队列

跟栈一样，`std::priority_queue` 没有实现 `begin()` 和 `end()` 方法，所以不能直接使用 `for` 循环遍历

但可以依次取出队列中的元素并进行处理，直到队列为空

```cpp
while (!queue.empty())
{
    std::cout << "Element: " << queue.top() << "\n";
    queue.pop();
}
```
