# STL 库

STL (Standard Template Library) 是 C++ 标准库的一部分，提供了许多模板类和算法

## 容器（Containers）

### [底层容器](底层容器/README.md)

底层容器包含序列容器、关联容器和无序容器

- **序列容器**
  - `vector`：动态数组
  - `deque`：双向队列
  - `list`：双向链表
  - `forward_list`：单向链表
  - `array`：固定大小数组
  - `implace_vector`：可变大小数组 (C++26新增)
  - `hive`: 蜂巢容器 (C++26新增)

- **关联容器**
  - `set`：集合
  - `multiset`：多重集合
  - `map`：映射
  - `multimap`：多重映射

- **无序容器**
  - `unordered_set`：无序集合
  - `unordered_multiset`：无序多重集合
  - `unordered_map`：无序映射
  - `unordered_multimap`：无序多重映射

### [容器适配器](容器适配器/README.md)

- `stack`：栈
- `queue`：队列
- `priority_queue`：优先队列
- `flat_map`: 平铺映射 (C++23新增)
- `flat_set`: 平铺集合 (C++23新增)

## 底层容器与容器适配器的区别

| 特性 | 底层容器 | 容器适配器 |
| :--: | :----: | :------: |
| 迭代器 | ✅ 支持 | ❌ 不提供 |
| 随机访问 | ✅ 部分支持 | ❌ 不提供 |
| 内存管理 | ✅ 直接管理 | ❌ 依赖底层容器 |
| 接口丰富度 | ✅ 完整操作 | ⚠️ 受限接口 |

- 容器适配器只有 `std::flat_*` 系列有迭代器和随机访问 (如 `begin() + n` 访问元素)，其他容器适配器没有迭代器和随机访问
  - `std_map` 还提供 `at()` 方法

## 迭代器（Iterators）

- `input_iterator`：输入迭代器
- `output_iterator`：输出迭代器
- `forward_iterator`：前向迭代器
- `bidirectional_iterator`：双向迭代器
- `random_access_iterator`：随机访问迭代器

## 算法（Algorithms）

### 非修改性算法

- `find`、`count`、`search`、`equal`

### 修改性算法

- `copy`、`transform`、`replace`、`fill`

### 排序与查找

- `sort`、`stable_sort`、`binary_search`、`lower_bound`、`upper_bound`

### 集合操作

- `merge`、`set_union`、`set_intersection`、`set_difference`

### 数值算法

- `accumulate`、`inner_product`、`partial_sum`