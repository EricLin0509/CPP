# 无序多重集合

无序多重集合 (`std::unordered_multiset`) 是无序容器的一种，它允许存储多个相同的元素，且不同元素是之间是无序的

相比 `std::unordered_set`，`std::unordered_multiset` 允许存储多个相同的元素

## 语法

需要引入 `unordered_set` 头文件

```cpp
#include <unordered_set>
```

### 声明

```cpp
std::unordered_multiset<元素类型> 集合名;
std::unordered_multiset<元素类型, 哈希函数> 集合名;
std::unordered_multiset<元素类型, 哈希函数, 相等比较函数> 集合名;
std::unordered_multiset<元素类型> 集合名 = {元素1, 元素2, 元素3, ...};
```

- 哈希函数：用于生成键的哈希值，如果未指定，默认使用 `std::hash` 函数
- 相等比较函数：用于比较键是否相等，如果未指定，默认使用 `std::equal_to` 函数

## 示例

现在声明一个 `std::unordered_multiset`，存储一些字符串

```cpp
std::unordered_multiset<std::string> unordered_multiset;
```

### 自定义哈希函数

有关哈希函数的详细信息，请参考[自定义哈希函数](../README.md#自定义哈希函数)

### 自定义相等比较函数

有关相等比较函数的详细信息，请参考[自定义比较函数](../README.md#自定义相等比较函数)

### 共同方法

有关所有无序容器都有的方法，请参考[无序容器的共同方法](../README.md#共同方法)

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
        bool operator==(const Employee& other) const // 使用 `unordered_multiset` 需要的元素必须实现 `operator==` 方法
        {
            return this->name == other.name && this->age == other.age;
        }

        friend struct std::hash<Employee>;
};

namespace std {
    template <>
    struct hash<Employee> {
        size_t operator()(const Employee& e) const noexcept // 特化 `std::hash` 以支持 `Employee` 类
        {
            size_t h1 = std::hash<std::string>{}(e.name);
            size_t h2 = std::hash<int>{}(e.age);
            return h1 ^ (h2 << 1);
        }
    };
}
```

```cpp
std::unordered_multiset<Employee> employees;
Employee emp1("John Doe", 1);
employees.insert(emp1);
employees.insert(std::move(emp1));
employees.emplace("Jane Doe", 3);
```

- 这里提供特化的 `std::hash<Employee>`，是因为 `std::unordered_multiset` 需要先计算哈希值，才能进行插入操作
    - 也可以提供[自定义哈希函数](#自定义哈希函数)
    - 有关特化的详细信息，请参考[模板特化](../../../../模板/模板特化/README.md)
- 这里提供 `operator==` 运算符重载，是因为 `std::unordered_multiset` 还需要额外处理哈希冲突
    - 也可以提供[自定义相等比较函数](#自定义相等比较函数)

通过输出可以看到

```bash
Constructor called # 这个是 emp1 的构造函数
Copy constructor called
Move constructor called
Constructor called # 这个是使用 emplace() 方法构造的对象的构造函数
```

使用 `emplace()` 方法时效率最高，因为不需要拷贝和移动对象

### `emplace_hint()` 方法

`emplace_hint()` 方法与 `emplace()` 方法类似，但需要提供一个迭代器作为插入位置的提示

```cpp
auto it = employees.find(Employee("John Doe", 1));
if (it != employees.end())
{
    employees.emplace_hint(it, "Bob", 2);
}
```

### `count()` 方法

使用 `count()` 方法可以检查集合中有多少个元素与给定值相同

```cpp
std::unordered_multiset<std::string> multiset;
multiset.insert("apple");
multiset.insert("apple");
multiset.insert("apple");
multiset.insert("banana");
multiset.insert("banana");
std::cout << multiset.count("apple") << std::endl; // 输出 3
std::cout << multiset.count("banana") << std::endl; // 输出 2
```

### 桶接口

有关桶接口的详细信息，请参考[桶接口](../README.md#桶接口)

### 散列策略

有关散列策略的详细信息，请参考[散列策略](../README.md#散列策略)

## 迭代器失效

有关迭代器失效的描述，请参考[迭代器失效](../../README.md#迭代器失效)
