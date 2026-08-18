# 单向链表

单向链表 (`std::forward_list`) 是 C++ STL 库中的一个序列容器，它使用单向链表结构来存储元素

`std::forward_list` 与 `std::vector` 不同，`std::forward_list` 中的元素在内存中是非连续存储的,每个节点都包含指向下一个节点的指针

## 与 `std::vector` 的区别

跟 [`std::list`](../双向链表/README.md#与-stdvector-的区别) 一样

- 相比 `std::list`，`std::forward_list` 有以下特点
    - 更小的内存占用 (没有前驱节点指针)
    - 无法反向遍历 (需要先反转单向链表)

## 内存

单向链表的是不连续的，每个元素都使用一个节点 (Node) 来存储，每个节点通过指针连接

以下是单向链表的内存布局简单示例 (C语言)

```c
struct Node {
    void *data; // 存储元素
    struct Node *next; // 指向下一个节点
};

struct Node *link_list = NULL; // 单向链表
```

## 语法

需要引入 `forward_list` 头文件

```cpp
#include <forward_list>
```

### 声明单向链表

```cpp
std::forward_list<类型> 链表名;
std::forward_list<类型> 链表名 = {元素1, 元素2, 元素3, ...};
std::forward_list<类型> 链表名(初始大小, 初始值);
```

- 初始值可以省略，如果省略会有以下行为
    - 对于基本类型会零初始化
    - 对于类类型会调用默认构造函数

## 示例

现在声明一个单向链表

```cpp
std::forward_list<int> list;
```

### 共同方法

有关所有序列容器都有的方法，请参考[序列容器的共同方法](../README.md#共同方法)

### `push` 系列方法

`std::forward_list` 只提供了 `push_front` 方法

```cpp
list.push_front(1); // 在链表头部插入元素
```

### `pop` 系列方法

`std::forward_list` 只提供了 `pop_front` 方法

```cpp
list.pop_front(); // 删除链表头部元素
```

#### `emplace_front()` 方法

`emplace_front()` 方法与 [`emplace()` 方法](../README.md#emplace-方法)类似，但只在容器头部插入元素

```cpp
list.emplace_front(1); // 在链表头部插入元素 1
```

> [!WARNING]
> `std::forward_list` 没有 `emplace_back()` 方法

### 交换两个单向链表

使用 `swap()` 方法交换两个单向链表

```cpp
std::forward_list<int> list1 = {1, 2, 3};
std::forward_list<int> list2 = {4, 5, 6};

list1.swap(list2); // 交换两个单向链表
```

### 对元素进行排序

跟[双向链表](../双向链表/README.md#对元素进行排序)一样，使用 `sort()` 方法对单向链表进行排序

### 合并两个有序列表

跟[双向链表](../双向链表/README.md#合并两个有序列表)一样，使用 `merge()` 方法合并两个有序的单向链表

### 删除连续的重复元素

跟[双向链表](../双向链表/README.md#删除连续的重复元素)一样，使用 `unique()` 方法删除连续的重复元素

### 反转元素的顺序

跟[双向链表](../双向链表/README.md#反转元素的顺序)一样，使用 `reverse()` 方法反转元素的顺序

### 满足特定条件时移除元素

跟[双向链表](../双向链表/README.md#满足特定条件时移除元素)一样，使用 `remove()` 方法或 `remove_if()` 方法满足特定条件时移除元素

## 迭代器失效

有关迭代器失效的描述，请参考[迭代器失效](../../README.md#迭代器失效)
