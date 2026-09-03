# 平铺映射

平铺映射 (`std::flat_map`) 是 C++23 新增的容器适配器，它将键值对分别存储在两个连续的内存块中，从而提供高效的带范围查找操作

平铺映射的键和值必须是可比较的，并且键必须是唯一的

## 与无序映射 `std::unordered_map` 的区别

| 特征 | `std::flat_map` | `std::unordered_map` |
| :--: | :-------------: | :-----------------: |
| 内存布局 | 连续内存布局 | 桶+链表布局 |
| 元素排列 | 严格有序 | 无序 |
| 查找 | O(log n) | O(1) |
| 插入/删除 | O(n) (需要移动元素) | O(1) |
| 内存开销 | 较小 | 较大 |
| 迭代器稳定性 | 不稳定 (插入/删除时失效) | 稳定 (只有元素被删除时失效) |

平铺映射适用场景:
- 极高的缓存效率
    - 对于小规模数据集 (N < 64)，其 `O(log n)` 搜索往往比无序映射的 `O(1)` 算法更快
- 无额外堆内存开销
    - 对于你插入的每一个键值对，它都不会进行单独的堆分配
- 编译期工具
    - 可轻松与 `std::array` 结合，在编译期构建完全不可变的 `constexpr` 查找表

无序映射适用场景:
- 处理海量数据集
    - 如果你需要存储数千个元素，`O(1)` 复杂度的查找操作能够轻松应对规模扩展，而以 `O(n)` 速度移动数组内存则会严重拖慢性能
- 指针稳定性
    - 指向元素的迭代器和引用保证保持有效，除非该特定元素被删除
- 重型对象
    - 避免在插入或删除操作期间，在内存中移动大量且昂贵的数据块

## 内存布局

以下是用C语言描述的平铺映射的内存布局

```c
struct flat_map {
    size_t size; // 映射的元素数量
    size_t capacity; // 映射的容量
    key_type* keys; // 键的数组
    value_type* values; // 值的数组
};
```

- `key` 和 `value` 的大小是相同的，在 C++ 中这两者都默认使用 `std::vector` 存储
- 这样的设计属于 SoA 模式 (Structure of Arrays)
    - 这样更容易让编译器进行自动映射化并生成带 [SIMD 指令](https://en.wikipedia.org/wiki/SIMD) (Single Instruction, Multiple Data) 的代码

## 语法

需要引入 `flat_map` 头文件

```cpp
#include <flat_map>
```

### 声明

```cpp
std::flat_map<键类型, 值类型> 映射名;
```

### 自定义比较函数

```cpp
std::flat_map<键类型, 值类型, 比较函数> 映射名;
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

## 语法

现在声明一个平铺映射，键为 `std::string`，值为 `int`

```cpp
std::flat_map<std::string, int> myMap;
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
std::flat_map<std::string, int, StringCmp> map;
```

#### 使用 lambda 表达式

使用 lambda 表达式也可以实现

```cpp
auto StringCmp = [](const std::string& a, const std::string& b) {
    return a < b;
};
std::flat_map<std::string, int, decltype(StringCmp)> map(StringCmp);
```

### 访问元素

有两种方式访问元素

- `[]` 索引操作符
- `at()` 方法


#### `[]` 索引操作符

`[]` 索引操作符用于访问映射中的元素，类似于数组

```cpp
map[0] = 1; // 将第一个元素设置为 1
```

#### `at()` 方法 

`at()` 方法用于访问映射中的元素

相比 `[]` 索引操作符，`at()` 方法会进行越界检查，如果越界会抛出 `std::out_of_range` 异常

所以推荐配合 `try-catch` 块使用

```cpp
try
{
    map.at(0) = 1; // 将第一个元素设置为 1
}
catch (const std::out_of_range& e)
{
    std::cout << "[ERROR] Index out of range!" << std::endl;
}
```

### 插入元素

使用 `insert()` 方法可以插入元素

它的返回值为 `std::pair<iterator, bool>`，其中 `iterator` 是插入位置的迭代器，`bool` 表示是否插入成功

```cpp
auto result = map.insert({"key", 100});
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

#### `insert_or_assign()` 方法

相比 `insert()`，`insert_or_assign()` 方法会插入或替换元素

```cpp
map.insert_or_assign("key", 100);
```

- 如果键已经存在，会替换元素
- 如果键不存在，会插入元素
- 返回值为插入或替换后的迭代器

### `emplace()` 方法

`emplace()` 系列相比 [`insert()` 方法](../README.md#插入元素)可以避免不必要的拷贝和移动操作

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
        }
};
```

```cpp
std::flat_map<int, Employee> employees;
Employee emp1("John Doe", 1);
employees.insert({1, emp1});
employees.insert({2, std::move(emp1)});
employees.emplace(
    std::piecewise_construct,
    std::forward_as_tuple(3),
    std::forward_as_tuple("Jane Doe", 2)
);
```

- 需要使用 `std::piecewise_construct` 和 `std::forward_as_tuple` 来构造对象

通过输出可以看到

```bash
Constructor called # 这个是 emp1 的构造函数
Copy constructor called # 对应 `insert()`
Move constructor called
Move constructor called
Move constructor called # 对应 `insert()` + 右值引用
Move constructor called
Move constructor called
Copy constructor called
Constructor called # 对应 `emplace()`
Move constructor called
Copy constructor called
Copy constructor called
```

使用 `emplace()` 方法时效率最高，但依旧需要移动操作

#### `try_emplace()` 方法

`try_emplace()` 方法与 `emplace()` 方法类似，但不需要配合 `std::piecewise_construct` 和 `std::forward_as_tuple` 来构造对象

同时 `try_emplace()` 提供了强异常安全保证，即使插入失败，参数也保持原样可用，而 `emplace()` 在失败时，右值引用参数可能已经被消耗

```cpp
employees.try_emplace(4, "Alice", 3);
```

### `emplace_hint()` 方法

`emplace_hint()` 方法与 `emplace()` 方法类似，但需要提供一个迭代器作为提示

它会向容器中尽可能接近紧接指定迭代器之前的位置插入新元素

```cpp
auto it = map.find("key");
if (it != map.end())
{
    map.emplace_hint(it, std::piecewise_construct,
        std::forward_as_tuple(5),
        std::forward_as_tuple("Bob", 4));
}
```

### `count()` 方法

使用 `count()` 方法可以检查映射中有多少个键与给定键相同

- 在 `std::flat_map` 中只可能返回 0 或 1
- 所以在 `std::flat_map` 更推荐使用 [`contains()`](#检查容器是否含有带特定键的元素) 方法

### 删除元素

使用 `erase()` 方法删除元素

```cpp
map.erase("key");
```

### 交换两个容器

使用 `swap()` 方法交换两个容器

```cpp
std::flat_map<std::string, int> map1 = {{"key", 100}, {"key2", 200}};
std::flat_map<std::string, int> map2 = {{"key3", 300}, {"key4", 400}};
map1.swap(map2);
```

### 查找元素

使用 `find()` 方法查找元素

```cpp
auto it = map.find("key");
if (it != map.end())
{
    std::cout << "Key found: " << it->first << " " << it->second << "\n";
}
else
{
    std::cout << "Key not found\n";
}
```

- 如果没有找到，返回的迭代器为 `map.end()`

### 带范围查找

有三种带范围查找的方法

- `lower_bound()`: 返回指向首个不小于 给定键的元素的迭代器
- `upper_bound()`: 返回指向首个大于 给定键的元素的迭代器
- `equal_range()`: 返回匹配特定键的元素范围

#### `lower_bound()` 方法

使用 `lower_bound()` 方法返回指向首个不小于 (即大于或等于) 给定键的元素的迭代器

```cpp
std::flat_map<int, std::string> numbers = {
    {0, "zero"},
    {1, "one"},
    {2, "two"},
    {3, "three"},
    {4, "four"}
};

auto it = numbers.lower_bound(2);
std::cout << "Lower bound: " << it->first << " " << it->second << "\n";
```

此时会输出

```bash
Lower bound: 2 two
```

#### `upper_bound()` 方法

使用 `upper_bound()` 方法返回指向首个大于 给定键的元素的迭代器

```cpp
auto it = numbers.upper_bound(2);
std::cout << "Upper bound: " << it->first << " " << it->second << "\n";
```

此时会输出

```bash
Upper bound: 3 three
```

#### `equal_range()` 方法

使用 `equal_range()` 方法返回容器中所有拥有给定键的元素的范围

一个指向首个不小于给定键的元素的迭代器，另一个指向首个大于给定键的元素的迭代器

相当于 `lower_bound()` 和 `upper_bound()` 的结合体

```cpp
auto range = numbers.equal_range(2);
std::cout << "Lower bound: " << range.first->first << " " << range.first->second << "\n";
std::cout << "Upper bound: " << range.second->first << " " << range.second->second << "\n";
```

此时会输出

```bash
Lower bound: 2 two
Upper bound: 3 three
```

### 检查容器是否含有带特定键的元素

使用 `contains()` (C++20) 方法检查容器是否含有带特定键的元素

```cpp
if (map.contains("key"))
{
    std::cout << "Key found\n";
}
else
{
    std::cout << "Key not found\n";
}
```

### 遍历容器

由于 `std::flat_map` 实现了 `begin()` 和 `end()` 方法，所以可以使用 `for` 循环遍历容器

```cpp
for (const auto& pair : map)
{
    std::cout << "{" << pair.first << ", " << pair.second << "}\n";
}
```

## 迭代器失效

有关迭代器失效的描述，请参考[迭代器失效](../../README.md#迭代器失效)
