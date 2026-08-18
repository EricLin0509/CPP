# 双向链表

双向链表 (`std::list`) 是 C++ STL 库中的一个序列容器，它基于双向循环链表实现

`std::list` 与 `std::vector` 不同，`std::list` 中的元素在内存中是非连续存储的,每个节点都包含指向前驱和后继节点的指针

## 与 `std::vector` 的区别

| 功能 | `std::vector` | `std::list` |
| :--: | :-----------: | :----------: |
| 内存布局 | 连续内存 | [非连续内存](#内存布局) |
| 随机访问 | O(1) | O(n) (需要遍历链表) |
| 缓存友好性 | 极好 | 极差 (各元素不在同一缓存行) |
| 在尾部插入/删除 | O(1) | O(1) |
| 在中间插入/删除 | O(n) (需要元素移动) | O(1)* (迭代器定位需要 O(n)) |
| 在头部插入/删除 | O(n) (需要元素移动) | O(1) |
| C-API 兼容性 | 极好 (可以使用 &vec[0] 传递) | 没有任何兼容 |

## 内存布局

双向链表的是不连续的，每个元素都使用一个节点 (Node) 来存储，每个节点通过指针 (双向链表为前驱和后继指针) 连接

以下是双向链表的内存布局简单示例 (C语言)

```c
/* 节点结构 */ 
struct Node {
    void *data; // 需要单独定义类型
    struct Node *prev;
    struct Node *next;
};

/* 双向链表结构 */ 
struct List {
    struct Node *head; // 头节点
    struct Node *tail; // 尾节点
    size_t size; // 元素个数
};
```

- 由于 `head` 和 `tail` 指针是双向链表的头和尾节点,因此在头部或尾部插入或删除元素时,只需要在 `head` 或 `tail` 指针上进行操作即可

## 语法

需要引入 `list` 头文件

```cpp
#include <list>
```

### 声明双向链表

```cpp
std::list<类型> 链表名;
std::list<类型> 链表名 = {元素1, 元素2, 元素3, ...};
std::list<类型> 链表名(初始大小, 初始值);
```

- 初始值可以省略，如果省略会有以下行为
    - 对于基本类型会零初始化
    - 对于类类型会调用默认构造函数

## 示例

现在声明一个双向链表

```cpp
std::list<int> list;
```

### 共同方法

有关所有序列容器都有的方法，请参考[序列容器的共同方法](../README.md#共同方法)

### `push` 系列方法

这里共有两种方法

- `push_front()`: 在双向链表头部插入元素
- `push_back()`: 在双向链表尾部插入元素

```cpp
list.push_front(1); // 在双向链表头部插入元素 1
list.push_back(2); // 在双向链表尾部插入元素 2
```

### `pop` 系列方法

这里共有两种方法

- `pop_front()`: 从双向链表头部删除元素
- `pop_back()`: 从双向链表尾部删除元素

```cpp
list.pop_front(); // 从双向链表头部删除元素
list.pop_back(); // 从双向链表尾部删除元素
```

#### `emplace_front()` 方法

`emplace_front()` 方法与 [`emplace()` 方法](../README.md#emplace-方法)类似，但只在容器头部插入元素

```cpp
list.emplace_front(1); // 在双向链表头部插入元素 1
```

### `emplace_back()` 方法

相比其他序列容器，双向链表提供了 `emplace_back()` 方法

`emplace_back()` 方法与 [`emplace()` 方法](../README.md#emplace-方法)类似，只是将元素直接插入到双向链表尾部

```cpp
list.emplace_back(1); // 在双向链表尾部插入元素 1
```

### 交换两个双向链表

使用 `swap()` 方法交换两个双向链表

```cpp
std::list<int> list1 = {1, 2, 3};
std::list<int> list2 = {4, 5, 6};

list1.swap(list2);
```

### 对元素进行排序

使用 `sort()` 方法对双向链表进行排序

```cpp
std::list<int> list = {1, 3, 2, 5, 4, 6};
list.sort(); // 对双向链表进行排序

for (int num : list)
{
    std::cout << num << " ";
}
std::cout << "\n";
```

- `sort()` 默认使用升序排序

此时会输出

```bash
1 2 3 4 5 6 
```

#### 提供比较函数

`sort()` 方法还可以提供一个比较函数作为参数，时间复杂度为 O(n log n)

如果提供比较函数，`sort()` 方法会使用该比较函数来**决定整个链表的排序顺序**

需要提供的函数原型如下

```cpp
bool cmp(const Type1 &a, const Type2 &b);
```

```cpp
bool cmp(const int& a, const int& b)
{
    return a > b; // 降序
}
```

例如实现降序排序

```cpp
std::list<int> list = {1, 3, 2, 5, 4, 6};
list.sort(cmp); // 降序排序

for (int num : list)
{
    std::cout << num << " ";
}
std::cout << "\n";
```

此时会输出

```bash
6 5 4 3 2 1 
```

### 合并两个有序列表

使用 `merge()` 方法合并两个有序列表

```cpp
std::list<int> list1 = {1, 3, 5};
std::list<int> list2 = {2, 4, 6};
list1.sort();
list2.sort();

list1.merge(list2); // 合并两个有序列表

for (int num : list1)
{
    std::cout << num << " "; // 输出 1 2 3 4 5 6
}
std::cout << "\n";
```

此时会输出

```bash
1 2 3 4 5 6 
```

- `merge()` 方法默认使用升序合并
- `merge()` 方法要求两个链表**必须是有序的**
    - 否则属于未定义行为

#### 提供排序函数

`merge()` 方法还可以提供一个排序函数作为参数

如果提供排序函数，`merge()` 方法会使用该排序函数来**决定整个链表的合并顺序**

需要提供的函数原型如下

```cpp
bool cmp(const Type1 &a, const Type2 &b);
```

例如实现降序合并

```cpp
bool cmp(const int& a, const int& b)
{
    return a > b; // 降序
}
```

```cpp
std::list<int> list1 = {1, 5, 3, 2, 4};
std::list<int> list2 = {7, 8, 6, 9, 10};
list1.sort(cmp);
list2.sort(cmp);

list1.merge(list2, cmp); // 降序合并

for (int num : list1)
{
    std::cout << num << " ";
}
std::cout << "\n";
```

此时会输出

```bash
10 9 8 7 6 5 4 3 2 1
```

### 删除连续的重复元素

使用 `unique()` 方法删除连续的重复元素

```cpp
std::list<int> list = {1, 2, 2, 3, 3, 4, 4, 4};
list.unique(); // 删除连续的重复元素

for (int num : list)
{
    std::cout << num << " "; // 输出 1 2 3 4
}
std::cout << "\n";
```

- `unique()` 方法只删除连续的重复元素,非连续的不处理

此时会输出

```bash
1 2 3 4 
```

### 反转元素的顺序

使用 `reverse()` 方法反转元素的顺序

```cpp
std::list<int> list = {1, 2, 3, 4, 5};
list.reverse(); // 反转元素的顺序

for (int num : list)
{
    std::cout << num << " "; // 输出 5 4 3 2 1
}
std::cout << "\n";
```

此时会输出

```bash
5 4 3 2 1 
```

### 满足特定条件时移除元素

有两种方法实现

- `remove()`: 当元素等于指定值时移除元素
- `remove_if()`: 当元素满足特定条件时移除元素

#### `remove()` 方法

```cpp
std::list<int> list = {1, 2, 3, 4, 5, 6};
list.remove(3); // 移除元素 3

for (int num : list)
{
    std::cout << num << " "; // 输出 1 2 4 5 6
}
std::cout << "\n";
```

此时会输出

```bash
1 2 4 5 6 
```

#### `remove_if()` 方法

```cpp
std::list<int> list = {1, 2, 3, 4, 5, 6};
list.remove_if([](int num){ return (num & 1) == 0; }); // 移除所有偶数

for (int num : list)
{
    std::cout << num << " "; // 输出 1 3 5
}
```

此时会输出

```bash
1 3 5 
```

## 迭代器失效

有关迭代器失效的描述，请参考[迭代器失效](../../README.md#迭代器失效)
