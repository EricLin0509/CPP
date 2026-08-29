# 无序容器

## 定义

无序容器是 C++ STL 库中的一种容器，它存储元素的顺序是不确定的

但可以通过键值进行快速查找。无序容器通常比顺序容器（如 `vector` 和 `list`）更快，因为它们使用哈希表来存储元素

## 与关联容器的区别

| 特征 | 关联容器 | 无序容器 |
| :--: | :----: | :-----: |
| 底层实现 | 红黑树 | 哈希表 |
| 元素顺序 | 有序 | 无序 |
| 平均插入/删除/查找时间复杂度 | O(log n) | O(1) |
| 最坏插入/删除/查找时间复杂度 | O(log n) | O(n) (因为哈希碰撞) |
| 键比较要求 | 需要 `operator<` (排序) | 需要哈希函数和相等比较 (`operator==`) |
| 内存开销 | 较小 | 较大 |

无序容器的适用场景
- 只需要根据精确的键来查找值 (例如，检查某个用户 ID 是否存在)
- 元素的具体顺序无关紧要
- 正在处理海量数据集，此时 O(1) 的查找速度能显著节省计算时间

关联容器的适用场景
- 需要键保持有序 (例如，按字母顺序打印词典)
- 需要查找元素范围 (例如，“查找键值在 10 到 50 之间的所有项”)
- 的键是极长的字符串，此时计算哈希值所耗费的时间可能反而超过了树结构比较所需的时间

## 无序容器的类型

- `unordered_set`：无序集合
- `unordered_multiset`：无序多重集合
- [`unordered_map`](无序映射/README.md)：无序映射
- [`unordered_multimap`](无序多重映射/README.md)：无序多重映射

## 自定义哈希函数

这里以 `unordered_map` 为例

无序容器需要哈希函数来生成键的哈希值，以便在哈希表中进行查找

可以使用自定义哈希函数来改变键的哈希方式

这里需要重写 `operator()` (函数调用运算符)，该方法原型如下

```cpp
template <class Key>
size_t operator()(const Key& key) const;
```

例如按照字符串的每一位进行左移，然后进行异或运算

```cpp
struct HashString {
    size_t operator()(const std::string& key) const {
        size_t hash = 0;
        for (int i = 0; i < key.length(); i++)
        {
            hash = (hash << i) ^ key[i];
        }
        return hash;
    }
};
```

- 这里需要使用结构体包裹哈希函数，因为哈希函数参数模板需要一个类型参数
- 这里的哈希函数需要非常好的唯一性，否则比较容易发生哈希碰撞，从而影响插入和删除操作

```cpp
std::unordered_map<std::string, int, HashString> unordered_map;
```

### 适用 lambda 表达式

也可以使用 lambda 表达式来定义哈希函数

```cpp
auto hash = [](const std::string& x) {
    size_t hash = 0;
    for (int i = 0; i < x.length(); i++)
    {
        hash = (hash << i) ^ x[i];
    }
    return hash;
}; // lambda 表达式

std::unordered_map<std::string, int, decltype(hash)> unordered_map(hash);
```

## 自定义相等比较函数

这里以 `unordered_map` 为例

可以使用自定义相等比较函数来改变键的比较方式

这里需要重写 `operator()` (函数调用运算符)，该方法原型如下

```cpp
template <class Key>
bool operator()(const Key& lhs, const Key& rhs) const;
```

例如比较两个字符串的内容

```cpp
struct CmpByContent {
    bool operator()(const std::string& lhs, const std::string& rhs) const {
        return lhs == rhs;
    }
};
```

- 这里需要使用结构体包裹相等比较函数，因为相等比较函数参数模板需要一个类型参数

```cpp
std::unordered_map<std::string, int, std::hash<std::string>, CmpByContent> unordered_map;
```

### 适用 lambda 表达式

也可以使用 lambda 表达式来定义相等比较函数

```cpp
auto cmp = [](const std::string& x, const std::string& y) {
    return x == y;
}; // lambda 表达式

std::unordered_map<std::string, int, std::hash<std::string>, decltype(cmp)> unordered_map(std::hash<std::string>{}, cmp);
```

## 共同方法

- [`empty()`](#检查是否为空) - 检查容器是否为空
- [`size()`](#获取大小) - 获取容器大小
- [`insert()`](#插入元素) - 插入元素
- [`find()`](#查找元素) - 查找元素
- [`erase()`](#删除元素) - 删除元素
- [`swap()`](#交换两个容器) - 交换两个容器
- [`contains()`](#检查容器是否含有带特定键的元素) - 检查容器是否含有带特定键的元素 (C++20)

### 检查是否为空

使用 `empty()` 方法可以检查容器是否为空

```cpp
std::unordered_map<std::string, int> unordered_map;
std::cout << "Container is empty: " << (std::unordered_map.empty() ? "true" : "false") << "\n";
```

### 获取大小

使用 `size()` 方法可以获取容器大小

```cpp
std::unordered_map<std::string, int> unordered_map = {
    {"apple", 1},
    {"banana", 2},
    {"cherry", 3}
};
std::cout << "Container size: " << unordered_map.size() << "\n";
```

### 插入元素

使用 `insert()` 方法插入元素

对于不同的关联容器，返回的值类型不同

- `std::unordered_map`/`std::unordered_set`: `std::pair<iterator, bool>`
    - 返回一个包含插入位置的迭代器和一个布尔值，表示插入是否成功
- `std::unordered_multimap`/`std::unordered_multiset`: `iterator`
    - 返回插入位置的迭代器 (因为允许重复键)

#### `std::unordered_map`/`std::unordered_set`

```cpp
std::unordered_map<std::string, int> map;
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

#### `std::unordered_multimap`/`std::unordered_multiset`

```cpp
std::unorder_multimap<std::string, int> multimap;
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
std::unordered_map<std::string, int> map1 = {{"key", 100}, {"key2", 200}};
std::unordered_map<std::string, int> map2 = {{"key3", 300}, {"key4", 400}};
map1.swap(map2);
```

### 查找元素

使用 `find()` 方法查找元素

```cpp
std::unordered_map<std::string, int> map = {{"key", 100}, {"key2", 200}};
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

## 桶接口

无序容器都使用了桶接口 (因为哈希表是基于桶来实现的)

它提供了以下方法

- [`bucket()`](#获取指定键的桶号) - 获取指定键的桶号
- [`bucket_count()`](#获取桶的数量) - 获取桶的数量
- [`max_bucket_count()`](#获取最大桶的数量) - 获取最大桶的数量

### 获取指定键的桶号

使用 `bucket()` 方法可以获取指定键的桶号

```cpp
std::unordered_map<std::string, int> map = {{"key", 100}, {"key2", 200}};
size_t bucket = map.bucket("key");
std::cout << "Bucket for key: " << bucket << "\n";
```

可能的输出

```bash
Bucket for key: 3
```

- 这里的输出不确定，由具体哈希函数决定

### 获取桶的数量

使用 `bucket_count()` 方法可以获取桶的数量

```cpp
std::unordered_map<std::string, int> map = {{"key", 100}, {"key2", 200}};
size_t bucket_count = map.bucket_count();
std::cout << "Number of buckets: " << bucket_count << "\n";
```

可能的输出

```bash
Number of buckets: 16
```

### 获取最大桶的数量

使用 `max_bucket_count()` 方法可以获取由于系统或库实现限制的能保有的最大桶数

```cpp
auto c1 = std::unordered_map<char, long>{};
auto c2 = std::unordered_map<long, long>{};

std::cout
    << "Max bucket count of\n" << std::hex << std::showbase
    << "c1: " << c1.max_bucket_count() << '\n'
    << "c2: " << c2.max_bucket_count() << '\n'
    ;
```

可能的输出

```bash
Max bucket count of
c1: 0xaaaaaaaaaaaaaaa
c2: 0xaaaaaaaaaaaaaaa
```

## 散列策略

散列策略 (Hashing Policy) 是指控制哈希表内部如何管理桶数量、负载因子以及何时进行重新哈希 (rehash) 的一组规则和参数

- [`load_factor()`](#获取负载因子) - 获取负载因子
- [`max_load_factor()`](#设置最大负载因子) - 设置最大负载因子
- [`rehash()`](#重新哈希) - 重新哈希
- [`reserve()`](#预留空间) - 预留空间

### 获取负载因子

使用 `load_factor()` 方法可以获取负载因子，即平均每桶元素数 (`size()` 除以 `bucket_count()`)

```cpp
std::unordered_map<std::string, int> map = {{"key", 100}, {"key2", 200}};
float load_factor = map.load_factor();
std::cout << "Load factor: " << load_factor << "\n";
```

### 设置最大负载因子

使用 `max_load_factor()` 方法可以获取或设置最大负载因子，即获取/设置触发 `rehash` 的阈值

当 `load_factor()` (元素数 / 桶数) 超过 `max_load_factor()` 时，容器会自动增加桶数量并重新哈希所有元素

修改默认值 (通常为 1.0) 会影响性能和内存使用，具体取决于应用场景

- 较小的负载因子：容器更早扩容，减少哈希碰撞，提高查找/插入效率 (接近 O(1))，但内存使用更多
- 较大的负载因子：允许更多元素挤在同一个桶里，减少桶的总数，节省内存但可能增加冲突链长度，降低性能

如果数据量很大且对性能敏感，可以适当调低 (如 0.75)，如果内存紧张，可以调高 (如 1.5 或更高)

```cpp
std::unordered_map<std::string, int> map = {{"key", 100}, {"key2", 200}};
std::cout << "Current max load factor: " << map.max_load_factor() << "\n";
map.max_load_factor(0.5f);
std::cout << "New max load factor: " << map.max_load_factor() << "\n";
```

### 重新哈希

使用 `rehash()` 用于手动触发哈希表的重新组织，即强制改变桶的数量

例如调用 `rehash(20)` 后，容器会将桶数量调整为至少 20 个，并将所有元素根据新的桶数量重新分配到对应的桶中

应用场景

- 预分配内存：如果你预先知道要插入大量元素，可以提前调用 `rehash()` 增加桶数，避免在插入过程中多次自动扩容和 `rehash`，从而提高性能
- 优化内存布局：在删除大量元素后，桶可能变得稀疏，调用 `rehash(0)` 或较小的值可以收缩桶数组，释放多余内存

### 与自动 `rehash` 的区别

- 自动 `rehash` 是在负载因子超过 `max_load_factor()` 时触发的
- 手动 `rehash()` 是立即执行的，不受负载因子限制

以下是自动 `rehash` 和手动 `rehash` 的性能对比——往容器中插入 100000000 个元素 (没错，有8个零)，然后删除 50000000 个元素 (没错，有7个零)

```cpp
// no_rehash.cpp

std::unordered_map<int, std::string> map; // 默认
for (int i = 0; i < 100000000; i++)
{
    map.insert({i, "Test Key"});
}

for (int i = 0; i < 50000000; i++)
{
    map.erase(i);
}
```

使用 `time` 命令测量执行时间

```bash
15.20s user 1.82s system 99% cpu 17.074 total
```

```cpp
// rehash.cpp

std::unordered_map<int, std::string> map;
map.rehash(100000000); // 设置至少 100000000 个桶，使得负载因子接近 1.0
for (int i = 0; i < 100000000; i++)
{
    map.insert({i, "Test Key"});
}

for (int i = 0; i < 50000000; i++)
{
    map.erase(i);
}
```

使用 `time` 命令测量执行时间

```bash
14.36s user 1.63s system 99% cpu 16.040 total
```

可以发现，使用手动 `rehash` 让时间减少了 0.71s

### 预留空间

使用 `reserve()` 方法可以预先分配空间，实际上调用 `rehash(std::ceil(count / max_load_factor()))`

例如调用 `reserve(1000)`，实际上调用 `rehash(std::ceil(1000 / max_load_factor()))`

#### 与 `rehash()` 的区别

- `rehash(n)`：要求桶的数量至少为 n。
- `reserve(n)`：要求容器能容纳至少 n 个元素而不触发 `rehash`

```cpp
std::unordered_map<int, std::string> map;
map.reserve(100000000); // 预留至少 100000000 个元素的空间
```

- 日常开发推荐用 `reserve()`，因为它更简单、不易出错。
- 只有在需要微调哈希表底层结构时才用 `rehash()`
