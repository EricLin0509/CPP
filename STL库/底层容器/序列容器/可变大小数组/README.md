# 可变大小数组

可变大小数组 (`std::implace_vector`) 是 C++26 新增的容器，用于存储不大于指定大小的元素

相比 `std::array`，它可以改变大小 (范围是`[0, N]`，其中 `N` 是容器的大小)

相比 `std::vector`，它是在栈上分配内存的，因此在内存使用上更高效

但容量是编译时固定的，不能动态改变

## 语法

需要引入 `implace_vector` 头文件

```cpp
#include <implace_vector>
```

### 声明

```cpp
std::implace_vector<类型, 容量大小> 变量名;
```

## 示例

现在声明一个可变大小数组，容量大小为 5

```cpp
std::implace_vector<int, 5> vec;
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

#### `try_push_back()` 方法

`try_push_back()` 方法跟 `push_back()` 类似，如果超过容量限制时无效果

```cpp
vec.try_push_back(3); // 尝试在向量尾部插入元素 3
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

#### `try_emplace_back()` 方法

`try_emplace_back()` 方法跟 `emplace_back()` 类似，如果超过容量限制时无效果

```cpp
vec.try_emplace_back(2); // 尝试在向量尾部插入元素 2
```

### 交换两个向量

使用 `swap()` 方法可以交换两个向量的内容

```cpp
std::implace_vector<int> vec1 = {1, 2, 3};
std::implace_vector<int> vec2 = {4, 5, 6};

vec1.swap(vec2); // 交换 vec1 和 vec2 的内容
```
