# 双向队列

双向队列 (`std::deque`) 是 STL 库中的一种底层容器

与 [队列 (`std::queue`)](../../容器适配器/队列/README.md) 不同，队列是容器适配器，需要一个底层容器来存储元素 (默认就是 `std::deque`)

与 [向量 (`std::vector`)](../向量/README.md) 相比，双向队列的内存布局是不连续的，因此在随机访问会比向量 (使用动态数组) 的效率低

## 与 `std::vector` 的区别

| 功能 | `std::vector` | `std::deque` |
| :--: | :-----------: | :----------: |
| 内存布局 | 连续内存 | [分段内存块](#内存布局) |
| 随机访问 | O(1) | O(1) 但稍慢 |
| 缓存友好性 | 极好 | 良好 (如果在同一个内存块内访问) |
| 在尾部插入/删除 | O(1) | O(1) |
| 在头部插入/删除 | O(n) (需要元素移动) | O(1) |
| C-API 兼容性 | 极好 (可以使用 &vec[0] 传递) | 没有任何兼容 |

## 内存布局

双向队列的内存布局是分段的，每个内存块都有一个固定大小，当内存块满了时，会创建一个新的内存块

以下是双向队列的内存布局简单示例 (C语言)

```c
#define CHUNK_SIZE 1024 // 每个内存块的大小为 1024 字节

struct Deque {
    void **map; // 指向内存块的指针数组
    size_t map_size; // 内存块指针数组的大小
    void *begin; // 指向第一个元素的指针
    void *end; // 指向最后一个元素的指针
};
```

- `map` 的实现机制与 `std::vector` 类似，都是动态数组，支持自动扩容

## 语法

需要引入 `deque` 头文件

```cpp
#include <deque>
```

### 声明双向队列

```cpp
std::deque<类型> 队列名;
std::deque<类型> 队列名 = {元素1, 元素2, 元素3, ...};
std::deque<类型> 队列名(初始大小, 初始值);
```

- 初始值可以省略，如果省略会有以下行为
    - 对于基本类型会零初始化
    - 对于类类型会调用默认构造函数

## 示例

现在声明一个双向队列

```cpp
std::deque<int> deque;
```

### 共同方法

有关所有序列容器都有的方法，请参考[序列容器的共同方法](../README.md#共同方法)

### 访问元素

有两种方式访问元素

- `[]` 索引操作符
- `at()` 方法


#### `[]` 索引操作符

`[]` 索引操作符用于访问双向队列中的元素，类似于数组

```cpp
deque[0] = 1; // 将第一个元素设置为 1
```

#### `at()` 方法 

`at()` 方法用于访问双向队列中的元素

相比 `[]` 索引操作符，`at()` 方法会进行越界检查，如果越界会抛出 `std::out_of_range` 异常

所以推荐配合 `try-catch` 块使用

```cpp
try
{
    deque.at(0) = 1; // 将第一个元素设置为 1
}
catch (const std::out_of_range& e)
{
    std::cout << "[ERROR] Index out of range!" << std::endl;
}
```

### `push` 系列方法

这里共有两种方法

- `push_front()`: 在队列头部插入元素
- `push_back()`: 在队列尾部插入元素

```cpp
deque.push_front(1); // 在队列头部插入元素 1
deque.push_back(2); // 在队列尾部插入元素 2
```

### `pop` 系列方法

这里共有两种方法

- `pop_front()`: 从队列头部删除元素
- `pop_back()`: 从队列尾部删除元素

```cpp
deque.pop_front(); // 从队列头部删除元素
deque.pop_back(); // 从队列尾部删除元素
```

#### `emplace_front()` 方法

`emplace_front()` 方法与 [`emplace()` 方法](../README.md#emplace-方法)类似，但只在容器头部插入元素

```cpp
deque.emplace_front(1); // 在队列头部插入元素 1
```

### `emplace_back()` 方法

相比其他序列容器，双向队列提供了 `emplace_back()` 方法

`emplace_back()` 方法与 [`emplace()` 方法](../README.md#emplace-方法)类似，只是将元素直接插入到队列尾部

```cpp
deque.emplace_back(1); // 在队列尾部插入元素 1
```

### 交换两个双向队列

使用 `swap()` 方法交换两个双向队列

```cpp
std::deque<int> deque1 = {1, 2, 3};
std::deque<int> deque2 = {4, 5, 6};

deque1.swap(deque2);
```

## 迭代器失效

有关迭代器失效的描述，请参考[迭代器失效](../../README.md#迭代器失效)

### 失效规则

| 操作 | 失效 |
| :--: | :--: |
| `push_front`/`push_back` | 所有迭代器可能失效 (包括 `end()`) |
| `emplace_front`/`emplace_back` | 所有迭代器可能失效 (包括 `end()`) |
| `insert`/`emplace` | 所有迭代器可能失效 (包括 `end()`) |
| `erase` | 如果在起始擦除——只有被擦除元素<br>如果在末尾擦除——只有被擦除元素和尾后迭代器<br>否则——所有迭代器 (包含尾后迭代器) |
| `clear` | 所有迭代器失效 |
| `pop_front`/`pop_back` | 被擦除元素和 + `end()` 失效 |
| `swap` | `end()` 可能失效 (由实现定义) |

- 可能失效是指不是每次调用都 100% 失效，而是说标准不保证任何迭代器保持有效，实现可以自由决定是否失效
