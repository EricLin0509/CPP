# 关联容器

## 定义

关联容器是 C++ STL 库中的一种容器，它存储键值对，并且键是唯一的

## 关联容器类型

- [`map`](映射/README.md): 映射
- `multimap`: 多重映射
- `set`: 集合
- `multiset`: 多重集合

## 自定义比较函数

这里使用 `std::map` 作为示例，其他关联容器类型类似

可以使用自定义比较函数来改变容器中键的排序方式

这里需要重写 `operator()` (函数调用运算符)，该方法原型如下

```cpp
template <class Key, class Value>
bool operator()(const Key& x, const Value& y) const;
```

这里比较两个字符串长度

```cpp
struct CmpByLength {
    bool operator()(const std::string& x, const std::string& y) const {
        return x.length() < y.length();
    }
};
```

- 这里需要使用结构体包裹比较函数，因为比较函数参数模板需要一个类型参数

```cpp
std::map<std::string, int, CmpByLength> map;
```

### 使用 lambda 表达式

使用 lambda 表达式也可以实现比较函数

```cpp
auto cmp = [](const std::string& x, const std::string& y) {
    return x.length() < y.length();
}; // lambda 表达式

std::map<std::string, int, decltype(cmp)> map(cmp);
```

## 共同方法

- [`empty()`](#检查是否为空) - 检查容器是否为空
- [`size()`](#获取大小) - 获取容器大小
- [`insert()`](#插入元素) - 插入元素
- [`find()`](#查找元素) - 查找元素
- [`erase()`](#删除元素) - 删除元素
- [`swap()`](#交换两个容器) - 交换两个容器
- [`lower_bound()`](#lower_bound-方法) - 返回指向首个不小于给定键的元素的迭代器
- [`upper_bound()`](#upper_bound-方法) - 返回指向首个大于给定键的元素的迭代器
- [`equal_range()`](#equal_range-方法) - 返回匹配特定键的元素范围

这里以 `std::map` 为例，其他关联容器类型类似

### 检查是否为空

使用 `empty()` 方法检查容器是否为空

```cpp
std::cout << (map.empty() ? "Empty" : "Not empty") << "\n";
```

### 获取大小

使用 `size()` 方法获取容器的大小

```cpp
std::cout << "Size: " << map.size() << "\n";
```

### 插入元素

使用 `insert()` 方法插入元素

对于不同的关联容器，返回的值类型不同

- `std::map`/`std::set`: `std::pair<iterator, bool>`
    - 返回一个包含插入位置的迭代器和一个布尔值，表示插入是否成功
- `std::multimap`/`std::multiset`: `iterator`
    - 返回插入位置的迭代器 (因为允许重复键)

#### `std::map`/`std::set`

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

#### `std::multimap`/`std::multiset`

```cpp
std::multimap<std::string, int> multimap;
auto result = multimap.insert({"key", 100});
```

### 删除元素

使用 `erase()` 方法删除元素

```cpp
map.erase("key");
```

### 交换两个容器

使用 `swap()` 方法交换两个容器

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

### 遍历容器

由于容器实现了 `begin()` 和 `end()` 方法，所以可以使用 `for` 循环遍历容器

```cpp
for (const auto& pair : map)
{
    std::cout << "{" << pair.first << ", " << pair.second << "}\n";
}
```
