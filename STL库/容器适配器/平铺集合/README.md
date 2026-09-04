# 平铺集合

平铺集合 (`std::flat_set`) 是 C++23 新增的容器适配器，它将元素存储在连续的内存块中，从而提供高效的带范围查找操作

平铺集合的元素必须是可比较的，并且键必须是唯一的

## 与无序集合 `std::unordered_set` 的区别

| 特征 | `std::flat_set` | `std::unordered_set` |
| :--: | :-------------: | :-----------------: |
| 内存布局 | 连续内存布局 | 桶+链表布局 |
| 元素排列 | 严格有序 | 无序 |
| 查找 | O(log n) | O(1) |
| 插入/删除 | O(n) (需要移动元素) | O(1) |
| 内存开销 | 较小 | 较大 |
| 迭代器稳定性 | 不稳定 (插入/删除时失效) | 稳定 (只有元素被删除时失效) |

平铺集合适用场景:
- 极高的缓存效率
    - 对于小规模数据集 (N < 64)，其 `O(log n)` 搜索往往比无序集合的 `O(1)` 算法更快
- 无额外堆内存开销
    - 对于你插入的每一个元素，它都不会进行单独的堆分配

无序集合适用场景:
- 处理海量数据集
    - 如果你需要存储数千个元素，`O(1)` 复杂度的查找操作能够轻松应对规模扩展，而以 `O(n)` 速度移动数组内存则会严重拖慢性能
- 指针稳定性
    - 指向元素的迭代器和引用保证保持有效，除非该特定元素被删除

## 内存布局

以下是用C语言描述的平铺集合的内存布局

```c
struct flat_set {
    size_t size; // 集合的元素数量
    size_t capacity; // 集合的容量
    key_type* elements; // 元素数组
};
```

- `elements` 在 C++ 中默认使用 `std::vector` 存储

## 语法

需要引入 `flat_set` 头文件

```cpp
#include <flat_set>
```

### 声明

```cpp
std::flat_set<元素类型> 集合名;
```

### 自定义比较函数

```cpp
std::flat_set<元素类型, 比较函数> 集合名;
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

- 如果不提供比较函数，则默认使用 `std::less` 进行比较

## 示例

现在声明一个平铺集合，元素为 `std::string`

```cpp
std::flat_set<std::string> mySet;
```
### 自定义比较函数

STL 要求比较器是一个可调用对象类型(函数对象、lambda等)，所以需要使用结构体包装 `operator()` 或使用 lambda 表达式

```cpp
struct StringCmp {
    bool operator()(const std::string& a, const std::string& b) const {
        return a < b;
    }
};
```

```cpp
std::flat_set<std::string, StringCmp> set;
```

#### 使用 lambda 表达式

使用 lambda 表达式也可以实现

```cpp
auto StringCmp = [](const std::string& a, const std::string& b) {
    return a < b;
};
std::flat_set<std::string, decltype(StringCmp)> set(StringCmp);
```

### 插入元素

使用 `insert()` 方法可以插入元素

它的返回值为 `std::pair<iterator, bool>`，其中 `iterator` 是插入位置的迭代器，`bool` 表示是否插入成功

```cpp
auto result = set.insert("key");
if (result.second)
{
    std::cout << "Element inserted successfully\n";
}
else
{
    std::cout << "Element already exists\n";
}
```

- 如果键已经存在，会插入失败

### `emplace()` 方法

`emplace()` 系列相比 [`insert()` 方法](#插入元素)可以避免不必要的拷贝和移动操作

这在配合对象构造函数时非常有用

```cpp
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
};
```

```cpp
std::flat_set<Employee> employees;
Employee emp1("John Doe", 1);
Employee emp2("James May", 2);
employees.insert(emp1);
employees.insert(std::move(emp2));
employees.emplace("Jane Doe", 2);
```

通过输出可以看到

```bash
Constructor called # 这个是 emp1 的构造函数
Constructor called # 这个是 emp2 的构造函数
Copy constructor called # 对应 `insert()`
Move constructor called # 对应 `insert()` + 右值引用
Copy constructor called
Constructor called # 对应 `emplace()`
```

使用 `emplace()` 方法时效率最高，但依旧需要移动或拷贝元素操作

### `emplace_hint()` 方法

`emplace_hint()` 方法与 `emplace()` 方法类似，但需要提供一个迭代器作为提示

它会向容器中尽可能接近紧接指定迭代器之前的位置插入新元素

```cpp
auto it = set.find(Employee("Jane Doe", 2));
if (it != set.end())
{
    set.emplace_hint(it, "Bob", 3);
}
```

### `count()` 方法

使用 `count()` 方法可以检查集合中有多少个元素与给定值相同

- 在 `std::flat_set` 中只可能返回 0 或 1
- 所以在 `std::flat_set` 更推荐使用 [`contains()`](#检查容器是否含有带特定键的元素) 方法

### 删除元素

使用 `erase()` 方法删除元素

```cpp
set.erase("key");
```

### 交换两个容器

使用 `swap()` 方法交换两个容器

```cpp
std::flat_set<std::string> set1 = {"key1", "key2"};
std::flat_set<std::string> set2 = {"key3", "key4"};
set1.swap(set2);
```

### 查找元素

使用 `find()` 方法查找元素

```cpp
auto it = set.find("key");
if (it != set.end())
{
    std::cout << "Element found: " << *it << "\n";
}
else
{
    std::cout << "Element not found\n";
}
```

- 如果没有找到，返回的迭代器为 `set.end()`

### 带范围查找

有三种带范围查找的方法

- `lower_bound()`: 返回指向首个不小于 给定键的元素的迭代器
- `upper_bound()`: 返回指向首个大于 给定键的元素的迭代器
- `equal_range()`: 返回匹配特定键的元素范围

#### `lower_bound()` 方法

使用 `lower_bound()` 方法返回指向首个不小于 (即大于或等于) 给定键的元素的迭代器

```cpp
std::flat_set<int> numbers = {0, 1, 2, 3, 4, 5};

auto it = numbers.lower_bound(2);
std::cout << "Lower bound: " << *it << "\n";
```

此时会输出

```bash
Lower bound: 2
```

#### `upper_bound()` 方法

使用 `upper_bound()` 方法返回指向首个大于 给定键的元素的迭代器

```cpp
auto it = numbers.upper_bound(2);
std::cout << "Upper bound: " << *it << "\n";
```

此时会输出

```bash
Upper bound: 3
```

#### `equal_range()` 方法

使用 `equal_range()` 方法返回容器中所有拥有给定键的元素的范围

一个指向首个不小于给定键的元素的迭代器，另一个指向首个大于给定键的元素的迭代器

相当于 `lower_bound()` 和 `upper_bound()` 的结合体

```cpp
auto range = numbers.equal_range(2);
std::cout << "Lower bound: " << *range.first << "\n";
std::cout << "Upper bound: " << *range.second << "\n";
```

此时会输出

```bash
Lower bound: 2
Upper bound: 3
```

### 检查容器是否含有带特定键的元素

使用 `contains()` (C++20) 方法检查容器是否含有带特定键的元素

```cpp
if (set.contains("key"))
{
    std::cout << "Key found\n";
}
else
{
    std::cout << "Key not found\n";
}
```

### 遍历容器

由于 `std::flat_set` 实现了 `begin()` 和 `end()` 方法，所以可以使用 `for` 循环遍历容器

```cpp
for (const auto& element : set)
{
    std::cout << element << "\n";
}
```

## 迭代器失效

迭代器失效是指在容器进行插入或删除操作时，迭代器指向的元素被删除或移动，导致迭代器失效

```cpp
std::flat_set<int> numbers = {1, 2, 3, 4, 5};

auto it = numbers.begin();
numbers.clear();
std::cout << *it << "\n"; // 这里属于未定义行为
```

- 如果在迭代器失效后继续使用该迭代器，可能会导致程序崩溃或产生不可预测的结果
