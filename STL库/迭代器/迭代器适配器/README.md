# 迭代器适配器

在上一章[迭代器操作](../迭代器操作/README.md)中，我们介绍了迭代器的操作方法

迭代器适配器是标准库提供的**包装器**，它将普通迭代器包装成具有特殊行为的迭代器

## 适配器分类

C++ STL 提供了以下几种迭代器适配器：

| 适配器 | 作用 | 辅助函数 |
| :---: | :---: | :------: |
| `std::reverse_iterator` | 反向遍历容器 | `rbegin()` / `rend()` |
| `std::back_insert_iterator` | 在容器尾部插入元素 | `std::back_inserter()` |
| `std::front_insert_iterator` | 在容器头部插入元素 | `std::front_inserter()` |
| `std::insert_iterator` | 在容器指定位置插入元素 | `std::inserter()` |
| `std::move_iterator` | 移动而非拷贝元素 | `std::make_move_iterator()` |


## 示例

### `std::reverse_iterator` — 反向迭代器

将正向迭代器包装为反向遍历的迭代器，`++` 操作实际上执行的是底层迭代器的 `--`

```cpp
std::vector<int> vec = {1, 2, 3, 4, 5};

// 方式一：直接使用容器的 rbegin/rend
for (auto it = vec.rbegin(); it != vec.rend(); ++it)
{
    std::cout << *it << " "; // 输出: 5 4 3 2 1
}

// 方式二：手动构造 reverse_iterator
auto rit = std::reverse_iterator<std::vector<int>::iterator>(vec.end());
auto rend = std::reverse_iterator<std::vector<int>::iterator>(vec.begin());
for (; rit != rend; ++rit)
{
    std::cout << *rit << " "; // 输出: 5 4 3 2 1
}
```

- `reverse_iterator` 的 `base()` 方法可以获取被包装的原始迭代器
    - 但注意 `*rit` 实际解引用的是 `--base()` 的位置

```cpp
// rit 指向 5，但 rit.base() 指向 5 的下一个位置（即 vec.end()）
// *rit 等于 *(rit.base() - 1)
auto rit = vec.rbegin();
std::cout << *rit << "\n"; // 输出: 5
std::cout << *(rit.base() - 1) << "\n"; // 输出: 5
```


### `std::back_insert_iterator` — 尾部插入迭代器

对赋值操作 `*it = val` 进行重定义，改为调用容器的 `push_back(val)`

**典型用途**：配合 `std::copy` 等算法向容器填充元素

```cpp
std::vector<int> src = {1, 2, 3};
std::vector<int> dest;

// 错误写法：dest 为空，std::copy 会越界
// std::copy(src.begin(), src.end(), dest.begin());

// 正确写法：使用 back_inserter 动态插入
std::copy(src.begin(), src.end(), std::back_inserter(dest)); // dest 现在为 {1, 2, 3}
```

#### 原理

```cpp
// back_insert_iterator 的核心行为
*it = val;  // 实际调用: container.push_back(val);
++it;       // 无操作（no-op），仅返回 *this
```

### `std::front_insert_iterator` — 头部插入迭代器

与 `back_insert_iterator` 类似，但调用 `push_front(val)`，因此**只适用于支持 `push_front` 的容器**（如 `std::deque`、`std::list`）

```cpp
std::list<int> src = {1, 2, 3};
std::deque<int> dest;

std::copy(src.begin(), src.end(), std::front_inserter(dest));
// dest 为 {3, 2, 1}（每次 push_front，顺序反转）
```

### `std::insert_iterator` — 通用插入迭代器

在容器的指定位置调用 `insert()`，适用于所有容器

```cpp
std::vector<int> src = {10, 20, 30};
std::vector<int> dest = {1, 2, 3};

auto it = std::inserter(dest, dest.begin() + 1); // 在位置 1 处插入
std::copy(src.begin(), src.end(), it);
// dest 为 {1, 10, 20, 30, 2, 3}
```

### `std::move_iterator` — 移动迭代器 (C++11)

将解引用操作从返回左值引用改为返回右值引用，即 `*it` 返回 `std::move(*base_it)`，实现移动语义而非拷贝

```cpp
std::vector<std::string> src = {"hello", "world"};
std::vector<std::string> dest;

// 使用 move_iterator 移动元素而非拷贝
dest.insert(dest.end(),
    std::make_move_iterator(src.begin()),
    std::make_move_iterator(src.end()));
// src 中的字符串已被移动，可能变为空字符串
// auto it = src.begin();
// std::cout << *it << "\n"; // 属于未定义行为
```

### 总结对比

| 适配器 | 赋值行为 | 适用容器 | 自增行为 |
| :---: | :------: | :------: | :------: |
| `reverse_iterator` | 解引用 `--base()` | 所有 | 底层 `--` |
| `back_insert_iterator` | `push_back(val)` | `vector`, `deque`, `list`, `string` | 无操作 |
| `front_insert_iterator` | `push_front(val)` | `deque`, `list`, `forward_list` | 无操作 |
| `insert_iterator` | `insert(pos, val)` | 所有 | 无操作 |
| `move_iterator` | `std::move(*base)` | 所有 | 底层 `++` |
