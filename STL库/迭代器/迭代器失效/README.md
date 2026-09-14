# 迭代器失效

在上一章[迭代器适配器](../迭代器适配器/README.md)中，我们介绍了迭代器适配器的用法

迭代器失效是指在对容器进行某些操作后，之前获取的迭代器不再指向有效的元素，此时使用该迭代器属于**未定义行为**

## 失效原因

迭代器失效的根本原因是容器的内部存储发生了变化，导致迭代器所持有的指针或索引不再有效

常见的失效原因：

- **内存重分配**：容器扩容时重新分配了内存，旧地址失效（如 `vector` 的 `push_back` 触发扩容）
- **元素移动**：插入或删除导致元素位置发生偏移（如 `vector` 中间插入/删除）
- **节点删除**：链表或树结构中节点被释放（如 `list` 的 `erase`）

## 各容器迭代器失效规则

### `std::vector`

| 操作 | 失效的迭代器 |
| :--: | :---------: |
| `push_back` | 若触发扩容，所有迭代器失效；否则仅 `end()` 失效 |
| `insert` | 若触发扩容，所有迭代器失效；否则插入点之后的迭代器失效 |
| `erase` | 被删除元素及其之后的迭代器失效 |
| `reserve` / `resize` | 若触发扩容，所有迭代器失效 |
| `clear` | 所有迭代器失效 |

```cpp
std::vector<int> vec = {1, 2, 3, 4, 5};
auto it = vec.begin() + 2; // 指向 3

vec.push_back(6); // 若未扩容，it 仍有效；若扩容，it 失效

vec.erase(vec.begin() + 1); // 删除 2，it 失效（因为 it 之后的元素移动了）
```

### `std::deque`

| 操作 | 失效的迭代器 |
| :--: | :---------: |
| `push_front` / `push_back` | 所有迭代器失效 |
| `insert` (中间) | 所有迭代器失效 |
| `erase` (中间) | 所有迭代器失效 |
| `erase` (首/尾) | 仅被删除元素的迭代器失效 |

```cpp
std::deque<int> dq = {1, 2, 3, 4, 5};
auto it = dq.begin() + 2; // 指向 3

dq.push_back(6); // it 失效！deque 的 push_back 会使所有迭代器失效
```

- `deque` 的失效规则比 `vector` 更严格，因为 `deque` 的分段存储结构在首尾插入时可能触发段的重新分配

### `std::list` / `std::forward_list`

| 操作 | 失效的迭代器 |
| :--: | :---------: |
| `insert` | 无迭代器失效 |
| `erase` | 仅指向被删除元素的迭代器失效 |
| `splice` | 被移动元素的迭代器仍有效，但指向的容器已改变 |
| `clear` | 所有迭代器失效 |

```cpp
std::list<int> lst = {1, 2, 3, 4, 5};
auto it = lst.begin();
std::advance(it, 2); // 指向 3

lst.push_back(6); // it 仍有效，list 的插入不会使其他迭代器失效

lst.erase(it); // it 失效，但其他迭代器仍有效
```

- 链表结构不会发生内存重分配和元素移动，因此失效规则最为宽松

### 关联容器 (`std::map` / `std::set` / `std::unordered_map` / `std::unordered_set`)

| 操作 | 失效的迭代器 |
| :--: | :---------: |
| `insert` | 无迭代器失效 |
| `erase` | 仅指向被删除元素的迭代器失效 |
| `clear` | 所有迭代器失效 |

- 对于 `unordered_map` / `unordered_set`，`insert` 若触发 rehash，所有迭代器失效

```cpp
std::map<int, std::string> m = {{1, "a"}, {2, "b"}, {3, "c"}};
auto it = m.find(2);

m.insert({4, "d"}); // it 仍有效

m.erase(it); // it 失效，但其他迭代器仍有效
```

## 安全删除模式

在遍历容器时删除元素是最常见的迭代器失效场景，以下是各容器的安全删除方式

### `std::vector` / `std::deque` — 使用返回值

`erase` 返回指向下一个有效元素的迭代器

```cpp
std::vector<int> vec = {1, 2, 3, 4, 5};
for (auto it = vec.begin(); it != vec.end(); )
{
    if (*it % 2 == 0)
    {
        it = vec.erase(it); // erase 返回下一个有效迭代器
    }
    else
    {
        ++it;
    }
}
// vec 为 {1, 3, 5}
```

### `std::list` / 关联容器 — 使用后置自增

利用后置 `it++` 返回删除前的副本的特性

```cpp
std::map<int, std::string> m = {{1, "a"}, {2, "b"}, {3, "c"}};
for (auto it = m.begin(); it != m.end(); )
{
    if (it->first % 2 == 0)
    {
        m.erase(it++); // it++ 先保存当前迭代器，再自增，erase 删除的是旧迭代器
    }
    else
    {
        ++it;
    }
}
```

### C++20 — `std::erase_if`

C++20 提供了统一的 `std::erase_if`，内部处理了迭代器失效问题

```cpp
std::vector<int> vec = {1, 2, 3, 4, 5};
std::erase_if(vec, [](int x) { return x % 2 == 0; });
// vec 为 {1, 3, 5}

std::map<int, std::string> m = {{1, "a"}, {2, "b"}, {3, "c"}};
std::erase_if(m, [](const auto& p) { return p.first % 2 == 0; });
// m 为 {{1, "a"}, {3, "c"}}
```

## 总结

| 容器 | 插入失效 | 删除失效 | 安全删除方式 |
| :--: | :------: | :------: | :---------: |
| `vector` | 可能全部失效 | 被删元素及之后失效 | `it = erase(it)` |
| `deque` | 可能全部失效 | 可能全部失效 | `it = erase(it)` |
| `list` / `forward_list` | 不失效 | 仅被删元素失效 | `erase(it++)` |
| `map` / `set` | 不失效 | 仅被删元素失效 | `erase(it++)` |
| `unordered_map` / `unordered_set` | rehash 时全部失效 | 仅被删元素失效 | `erase(it++)` |
