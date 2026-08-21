# 映射

映射 (`std::map`) 是一个关联容器，它存储键值对，并且键是唯一的

它采用红黑树作为底层实现

## 语法

需要引入 `map` 头文件

```cpp
#include <map>
```

### 声明

```cpp
std::map<键类型, 值类型> 映射名;
std::map<键类型, 值类型, 比较函数> 映射名;
std::map<键类型, 值类型> 映射名 = {
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
std::map<std::string, int> map;
```
### 自定义比较函数

有关比较函数的详细信息，请参考[自定义比较函数](../README.md#自定义比较函数)

### 检查是否为空

使用 `empty()` 方法检查映射是否为空

```cpp
std::cout << (map.empty() ? "Empty" : "Not empty") << "\n";
```

### 获取大小

使用 `size()` 方法获取映射的大小

```cpp
std::cout << "Size: " << map.size() << "\n";
```

### 访问元素

有两种方法可以访问元素

- 使用 `[]` 操作符
- 使用 `at()` 方法

#### 使用 `[]` 操作符

```cpp
map.insert({"key", 100});
std::cout << map["key"] << "\n";
```

#### 使用 `at()` 方法

相比 `[]` 操作符，使用 `at()` 方法时未找到键时会抛出 `std::out_of_range` 异常

所以推荐配合 `try-catch` 块使用

```cpp
try
{
    std::cout << map.at("key") << "\n";
}
catch (const std::out_of_range& e)
{
    std::cout << "Key not found: " << e.what() << "\n";
}
```

### 插入元素

使用 `insert()` 方法插入元素

相比序列容器的 [`insert()` 方法](../../序列容器/README.md#insert-方法)，映射的 `insert()` 方法返回一个迭代器和一个布尔值 (使用 `std::pair` 包裹)

具体的返回值类型如下

```cpp
std::pair<iterator, bool>
```

```cpp
auto result = map.insert({"key", 100});
if (result.second)
{
    std::cout << "Key inserted successfully\n";
}
else
{
    std::cout << "Key already exists\n";
}
```

- 如果插入成功，返回的迭代器有效且布尔值为 `true`
- 如果插入失败，返回的迭代器无效且布尔值为 `false`
    - 通常是因为键已经存在

#### `insert_or_assign()` 方法

在 C++17 中，`std::map` 提供了 `insert_or_assign()` 方法，用于插入元素或替换元素

```cpp
map.insert_or_assign("key", 100);
```

- 如果键已经存在，会替换元素
- 如果键不存在，会插入元素
- 返回值为插入或替换后的迭代器

### `emplace()` 方法

`emplace()` 系列相比 `insert()` 方法可以避免不必要的拷贝和移动操作

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
std::map<int, Employee> emp_map;
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

#### 使用 `try_emplace()` 方法

在 C++17 中引入了 `try_emplace()` 方法

`try_emplace()` 方法与 `emplace()` 方法类似，但不需要配合 `std::piecewise_construct` 和 `std::forward_as_tuple` 来构造对象

```cpp
emp_map.try_emplace(4, "Alice", 3);
```

### `emplace_hint()` 方法

`emplace_hint()` 方法与 `emplace()` 方法类似，但需要提供一个迭代器作为提示

它会向容器中尽可能接近紧接指定迭代器之前的位置插入新元素

```cpp
auto it = map.find("key");
if (it != map.end())
{
    map.emplace_hint(it, "key2", 3);
}
```

### 删除元素

使用 `erase()` 方法删除元素

```cpp
map.erase("key");
```

### 交换两个映射

使用 `swap()` 方法交换两个映射

```cpp
std::map<std::string, int> map1 = {{"key", 100}, {"key2", 200}};
std::map<std::string, int> map2 = {{"key3", 300}, {"key4", 400}};
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
std::map<int, std::string> numbers = {
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

### 遍历映射

由于映射实现了 `begin()` 和 `end()` 方法，所以可以使用 `for` 循环遍历映射

```cpp
for (const auto& pair : map)
{
    std::cout << "{" << pair.first << ", " << pair.second << "}\n";
}
```

## 迭代器失效

有关迭代器失效的描述，请参考[迭代器失效](../../README.md#迭代器失效)
