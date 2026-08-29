# 无序多重映射

无序多重映射 (`std::unordered_multimap`) 是 C++ 标准库中的一个容器，用于存储键值对

相比 `std::unordered_map`，`std::unordered_multimap` 允许一个键对应多个值

## 与 `std::unordered_map` 的区别

| 特征 | `std::unordered_map` | `std::unordered_multimap` |
| :--: | :-----------------: | :-----------------------: |
| 键唯一性 | 是 | 否 |
| 键值对数量 | 一个键对应一个值 | 一个键对应多个值 |
| 索引操作符 `[]` | 支持 | 不支持 |
| `insert()` 行为 | 键存在时插入失败 | 插入新键值对 |
| `count()` 行为 | 只可能返回 0 或 1 | 返回键对应的值的数量 |
| `erase()` 行为 | 删除键对应的唯一值 | 删除键对应的所有值 |

## 语法

需要引入 `unordered_map` 头文件

```cpp
#include <unordered_map>
```

### 声明

```cpp
std::unordered_multimap<键类型, 值类型> 映射名;
std::unordered_multimap<键类型, 值类型, 哈希函数> 映射名;
std::unordered_multimap<键类型, 值类型, 哈希函数, 相等比较函数> 映射名;
std::unordered_multimap<键类型, 值类型> 映射名 = {
    {键1, 值1},
    {键2, 值2},
    // ...
};
```

- 哈希函数：用于生成键的哈希值，如果未指定，默认使用 `std::hash` 函数
- 相等比较函数：用于比较键的是否相等，如果未指定，默认使用 `std::equal_to` 函数

## 示例

现在声明一个 `std::unordered_multimap`，存储键为字符串，值为整数的映射

```cpp
std::unordered_multimap<std::string, int> unordered_multimap;
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
};
```

```cpp
std::unordered_multimap<int, Employee> employees;
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

> [!WARNING]
> `std::unordered_multimap` 没有 `try_emplace()` 方法
> 因为 `try_emplace` 的目的是仅在键不存在时有条件地插入元素，从而避免在插入失败时销毁参数资源
> 而 `std::unordered_multimap` 允许多个相同的键

### `emplace_hint()` 方法

`emplace_hint()` 方法与 `emplace()` 方法类似，但需要提供一个迭代器作为插入位置的提示

```cpp
auto it = employees.find(5);
if (it != employees.end())
{
    employees.emplace_hint(it,
        std::piecewise_construct,
        std::forward_as_tuple(6),
        std::forward_as_tuple("Bob", 4)
    );
}
```

### `count()` 方法

使用 `count()` 方法可以检查映射中有多少个键与给定键相同

```cpp
std::unordered_multimap<int, std::string> map;
map.insert({1, "Key1"});
map.insert({1, "Key1"});
map.insert({1, "Key1"});
map.insert({2, "Key2"});
map.insert({2, "Key2"});

std::cout << "Key 1 appears " << map.count(1) << " times\n"; // 输出: Key 1 appears 3 times
std::cout << "Key 2 appears " << map.count(2) << " times\n"; // 输出: Key 2 appears 2 times
```


### 桶接口

有关桶接口的详细信息，请参考[桶接口](../README.md#桶接口)

### 散列策略

有关散列策略的详细信息，请参考[散列策略](../README.md#散列策略)

## 迭代器失效

有关迭代器失效的描述，请参考[迭代器失效](../../README.md#迭代器失效)
