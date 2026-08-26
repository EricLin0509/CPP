# 多重集合

多重集合 (`std::multiset`) 是关联容器

相比 `std::set`，`std::multiset` 允许存储重复的元素

## 与 `std::set` 的区别

| 特征 | `std::set` | `std::multiset` |
| :--: | :-------: | :-------------: |
| 键唯一性 | 是 | 否 |
| `insert()` 行为 | 键存在时插入失败 | 插入新键 |
| `count()` 行为 | 只可能返回 0 或 1 | 返回键出现的次数 |
| `erase()` 行为 | 删除唯一键 | 删除所有键 |

## 语法

需要引入 `set` 头文件

```cpp
#include <set>
```

### 声明

```cpp
std::multiset<元素类型> 集合名;
std::multiset<元素类型, 比较函数> 集合名;
std::multiset<元素类型> 集合名 = {
    值1, 值2, 值3, ...
};
```

- 比较函数可以省略，如果省略，默认使用 `std::less` 函数作为比较函数
- 如果使用自定义比较函数，需要使用结构体包裹比较函数，因为第三个模板参数需要一个类型参数

## 示例

现在声明一个多重集合

```cpp
std::multiset<int> multi_set = {1, 2, 3, 4, 5};
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

        bool operator<(const Employee& other) const // multiset 需要重载 `<` 运算符
        {
            return age < other.age;
        }
};
```

```cpp
std::multiset<Employee> employees;
Employee alice("Alice", 30);
employees.insert(std::move(alice));
employees.emplace("Alice", 30);
```

- 这里提供 `operator<` 运算符重载，是因为 `std::multiset` 中的元素必须是可比较的
    - 也可以提供[自定义比较函数](#自定义比较函数)

通过输出可以看到

```bash
Constructor called # 这个是 alice 的构造函数
Move constructor called
Constructor called # 这个是使用 emplace() 方法构造的对象的构造函数
```

使用 `emplace()` 方法时效率最高，因为不需要拷贝和移动对象

### `emplace_hint()` 方法

`emplace_hint()` 方法与 `emplace()` 方法类似，但需要提供一个迭代器作为提示

它会向容器中尽可能接近紧接指定迭代器之前的位置插入新元素

```cpp
auto it = employees.find(Employee("Alice", 30));
if (it != employees.end())
{
    employees.emplace_hint(it, "James", 25);
}
```

### `count()` 方法

使用 `count()` 方法可以返回给定元素在多重集合中出现的次数

```cpp
multi_set.insert(1);
multi_set.insert(1);
multi_set.insert(1);
multi_set.insert(2);
multi_set.insert(2);

std::cout << multi_set.count(1) << std::endl; // 输出 3
std::cout << multi_set.count(2) << std::endl; // 输出 2
```

## 迭代器失效

有关迭代器失效的描述，请参考[迭代器失效](../../README.md#迭代器失效)
