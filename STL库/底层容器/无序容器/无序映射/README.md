# 无序映射

无序映射 (`std::unordered_map`) 是无序容器的一种，它存储键值对，键是唯一的，值可以重复

相比 `std::map`, `std::unordered_map` 的底层实现是哈希表，因此查找、插入和删除操作的时间复杂度是O(1)

## 语法

需要引入 `unordered_map` 头文件

```cpp
#include <unordered_map>
```

### 声明

```cpp
std::unordered_map<键类型, 值类型> 映射名;
std::unordered_map<键类型, 值类型, 哈希函数> 映射名;
std::unordered_map<键类型, 值类型, 哈希函数, 相等比较函数> 映射名;
std::unordered_map<键类型, 值类型> 映射名 = {
    {键1, 值1},
    {键2, 值2},
    // ...
};
```

- 哈希函数：用于生成键的哈希值，如果未指定，默认使用 `std::hash` 函数
- 比较函数：用于比较键的大小，如果未指定，默认使用 `std::equal_to` 函数

## 示例

现在声明一个 `std::unordered_map`，存储键为字符串，值为整数的映射

```cpp
std::unordered_map<std::string, int> unordered_map;
```

### 自定义哈希函数

有关哈希函数的详细信息，请参考[自定义哈希函数](../README.md#自定义哈希函数)

### 自定义相等比较函数

有关相等比较函数的详细信息，请参考[自定义比较函数](../README.md#自定义相等比较函数)

### 共同方法

有关所有无序容器都有的方法，请参考[无序容器的共同方法](../README.md#共同方法)

### 访问元素

有两种方法可以访问元素

- 使用 `[]` 操作符
- 使用 `at()` 方法

#### 使用 `[]` 操作符

```cpp
unordered_map.insert({"key", 100});
std::cout << unordered_map["key"] << "\n";
```

#### 使用 `at()` 方法

相比 `[]` 操作符，使用 `at()` 方法时未找到键时会抛出 `std::out_of_range` 异常

所以推荐配合 `try-catch` 块使用

```cpp
try
{
    std::cout << unordered_map.at("key") << "\n";
}
catch (const std::out_of_range& e)
{
    std::cout << "Key not found: " << e.what() << "\n";
}
```

#### `insert_or_assign()` 方法

在 C++17 中，`std::unordered_map` 提供了 `insert_or_assign()` 方法，用于插入元素或替换元素

```cpp
unordered_map.insert_or_assign("key", 100);
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
};
```

```cpp
std::unordered_map<int, Employee> employees;
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
Copy constructor called
Move constructor called
Move constructor called
Move constructor called
Constructor called # 这个是使用 emplace() 方法构造的对象的构造函数
```

使用 `emplace()` 方法时效率最高，因为不需要拷贝和移动对象

#### `try_emplace()` 方法

在 C++17 中引入了 `try_emplace()` 方法

`try_emplace()` 方法与 `emplace()` 方法类似，但不需要配合 `std::piecewise_construct` 和 `std::forward_as_tuple` 来构造对象

同时 `try_emplace()` 提供了强异常安全保证，即使插入失败，参数也保持原样可用，而 `emplace()` 在失败时，右值引用参数可能已经被消耗

```cpp
const char *name = "Alice";
auto result = employees.try_emplace(4, name, 3);
if (!result.second)
{
    // 插入失败，但 name 未被消耗，仍可继续使用
    std::cout << "Insertion failed, name is still: " << name << "\n";
}
```

### `emplace_hint()` 方法

`emplace_hint()` 方法与 `emplace()` 方法类似，但需要提供一个迭代器作为插入位置的提示

```cpp
auto it = employees.find(5);
if (it != employees.end())
{
    employees.emplace_hint(it, 6, "Bob", 4);
}
```

### `count()` 方法

使用 `count()` 方法可以检查映射中有多少个键与给定键相同

- 在 `std::unordered_map` 中只可能返回 0 或 1
- 所以在 `std::unordered_map` 更推荐使用 `contains()` (C++20) 方法

### 桶接口

有关桶接口的详细信息，请参考[桶接口](../README.md#桶接口)

### 散列策略

有关散列策略的详细信息，请参考[散列策略](../README.md#散列策略)

## 迭代器失效

有关迭代器失效的描述，请参考[迭代器失效](../../README.md#迭代器失效)