# 底层容器

底层容器是 C++ STL 库中实现容器的底层数据结构

## 主要特点

- 提供完整接口：支持迭代器、随机访问 (**部分**容器支持，如 `vector`)
- 直接管理内存：负责元素的存储和销毁
- 功能丰富：提供插入、删除、查找等多种操作

## 底层容器分类

底层容器包含序列容器、关联容器和无序容器

- **序列容器**
  - [`vector`](向量/README.md)：动态数组
  - [`deque`](双向队列/README.md)：双向队列
  - `list`：双向链表
  - `forward_list`：单向链表
  - `array`：固定大小数组

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
