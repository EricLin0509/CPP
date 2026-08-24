# 多重映射

多重映射 (`std::multimap`) 是关联容器，它允许存储键值对

相比 `std::map`，`std::multimap` 允许存储多个键相同的值

它采用红黑树作为底层实现

## 与 `std::map` 的区别

| 特征 | `std::map` | `std::multimap` |
| :--: | :-------: | :-------------: |
| 键唯一性 | 是 | 否 |
| 键值对数量 | 一个键对应一个值 | 一个键对应多个值 |
| 索引操作符 `[]` | 支持 | 不支持 |
| `insert()` 行为 | 键存在时插入失败 | 插入新键值对 |
| `count()` 行为 | 只可能返回 0 或 1 | 返回键对应的值的数量 |
| `erase()` 行为 | 删除键对应的唯一值 | 删除键对应的所有值 |

## 语法

需要引入 `map` 头文件

```cpp
#include <map>
```

### 声明

```cpp
std::multimap<键类型, 值类型> 映射名;
std::multimap<键类型, 值类型, 比较函数> 映射名;
std::multimap<键类型, 值类型> 映射名 = {
    {键1, 值1},
    {键2, 值2},
    ...
};
```

- 键类型可以与值类型相同
- 比较函数可以省略，如果省略，默认使用 `std::less` 函数作为比较函数
- 如果使用自定义比较函数，需要使用结构体包裹比较函数，因为第三个模板参数需要一个类型参数

## 示例

现在声明一个映射

```cpp
std::multimap<int, std::string> multi_map;
```

### 自定义比较函数

有关比较函数的详细信息，请参考[自定义比较函数](../README.md#自定义比较函数)

### 共同方法

有关所有关联容器都有的方法，请参考[关联容器的共同方法](../README.md#共同方法)

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
std::multimap<int, Employee> emp_map;
Employee emp1("John Doe", 1);
emp_map.insert({1, emp1});
emp_map.insert({2, std::move(emp1)});
emp_map.emplace(
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
> `std::multimap` 没有 `try_emplace()` 方法
> 因为 `try_emplace` 的目的是仅在键不存在时有条件地插入元素，从而避免在插入失败时销毁参数资源
> 而 `std::multimap` 允许多个相同的键

### `emplace_hint()` 方法

`emplace_hint()` 方法与 `emplace()` 方法类似，但需要提供一个迭代器作为提示

它会向容器中尽可能接近紧接指定迭代器之前的位置插入新元素

```cpp
auto it = emp_map.find("key");
if (it != emp_map.end())
{
    emp_map.emplace_hint(it, std::piecewise_construct,
        std::forward_as_tuple(4),
        std::forward_as_tuple("James May", 3));
}
```

### `count()` 方法

使用 `count()` 方法可以检查映射中有多少个键与给定键相同

```cpp
multi_map.insert({1, "Alice"});
multi_map.insert({1, "Bob"});
multi_map.insert({1, "Jame"});
multi_map.insert({2, "Charlie"});
multi_map.insert({2, "David"});

std::cout << "Count of key 1: " << multi_map.count(1) << std::endl; // 输出 3
std::cout << "Count of key 2: " << multi_map.count(2) << std::endl; // 输出 1
```

## 迭代器失效

有关迭代器失效的描述，请参考[迭代器失效](../../README.md#迭代器失效)
