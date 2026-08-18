# 向量

向量 (Vector) 是 C++ 标准库中的一个容器，用于存储一组元素

相比数组，向量是可动态扩展的，可以自动调整大小

## 语法

要使用向量，需要包含头文件 `<vector>`，并使用 `std::vector` 类型

```cpp
#include <vector>
```

### 创建向量

```cpp
std::vector<类型> 向量名;
std::vector<类型> 向量名 = {元素1, 元素2, 元素3, ...};
std::vector<类型> 向量名(初始大小, 初始值);
```

- 初始值可以省略，如果省略会有以下行为
    - 对于基本类型会零初始化
    - 对于类类型会调用默认构造函数

## 示例

现在声明一个向量

```cpp
std::vector<int> vec;
```

### 共同方法

有关所有序列容器都有的方法，请参考[序列容器的共同方法](../README.md#共同方法)

### 访问元素

有两种方式访问元素

- `[]` 索引操作符
- `at()` 方法


#### `[]` 索引操作符

`[]` 索引操作符用于访问向量中的元素，类似于数组

```cpp
vec[0] = 1; // 将第一个元素设置为 1
```

#### `at()` 方法 

`at()` 方法用于访问向量中的元素

相比 `[]` 索引操作符，`at()` 方法会进行越界检查，如果越界会抛出 `std::out_of_range` 异常

所以推荐配合 `try-catch` 块使用

```cpp
try
{
    vec.at(0) = 1; // 将第一个元素设置为 1
}
catch (const std::out_of_range& e)
{
    std::cout << "[ERROR] Index out of range!" << std::endl;
}
```

### `push` 系列方法

向量只有 `push_back()` 方法

- `push_back()`: 在向量尾部插入元素

```cpp
vec.push_back(2); // 在向量尾部插入元素 2
```

### `pop` 系列方法

向量只有 `pop_back()` 方法

- `pop_back()`: 从向量尾部删除元素

```cpp
vec.pop_back(); // 从向量尾部删除元素
```

### `emplace_back()` 方法

相比其他序列容器，向量提供了 `emplace_back()` 方法

`emplace_back()` 方法与 [`emplace()` 方法](../README.md#emplace-方法)类似，只是将元素直接插入到向量尾部

```cpp
vec.emplace_back(1); // 在向量尾部插入元素 1
```

### 交换两个向量

使用 `swap()` 方法可以交换两个向量的内容

```cpp
std::vector<int> vec1 = {1, 2, 3};
std::vector<int> vec2 = {4, 5, 6};

vec1.swap(vec2); // 交换 vec1 和 vec2 的内容
```

### 查看当前存储空间能够容纳的元素数

使用 `capacity()` 方法可以查看当前存储空间能够容纳的元素数

- **注意不要**与 `size()` 方法混淆
    - `size()` 方法返回当前向量中实际存储的元素数
    - `capacity()` 方法返回当前存储空间能够容纳的元素数

```cpp
std::vector<int> vec = {1, 2, 3};
std::cout << "Capacity: " << vec.capacity() << "\n";
```

### 预留存储空间

由于 `vector` 每次添加数据时，会先检查存储空间是否足够，如果不够会先分配新的存储空间，然后将旧的数据复制到新的存储空间中

对于小量数据还行，但对于海量数据，频繁的分配和复制存储空间会严重影响性能

因此，可以使用 `reserve()` 方法预先分配存储空间

```cpp
std::vector<int> vec;
vec.reserve(1000); // 预留 1000 个元素的空间

vec.push_back(1); // 添加元素
vec.push_back(2); // 添加元素
vec.push_back(3); // 添加元素

std::cout << "Size: " << vec.size() << "\n";
std::cout << "Capacity: " << vec.capacity() << "\n";
```

此时的 `size()` 和 `capacity()` 的值分别为 `3` 和 `1000`

```bash
Size: 3
Capacity: 1000
```

## 迭代器失效

有关迭代器失效的描述，请参考[迭代器失效](../../README.md#迭代器失效)

### 失效规则

| 操作 | 失效 |
| :--: | :--: |
| 直接赋值 (`operator=`) | 所有迭代器失效 |
| `push_back` | vector 更改容量时全部失效，否则只有 `end()` |
| `emplace_back` | vector 更改容量时全部失效，否则只有 `end()` |
| `insert`/`emplace` | vector 更改容量时全部失效，否则只有插入点之后的迭代器失效 (包括 `end()`) |
| `erase` | 被擦除元素及之后的所有元素 (包括 `end()`) |
| `clear` | 所有迭代器失效 |
| `pop_back` | 被擦除元素和 `end()` 失效 |
| `swap` | `end()` 失效 |
