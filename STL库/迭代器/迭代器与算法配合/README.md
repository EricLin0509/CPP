# 迭代器与算法配合

在上一章[迭代器失效](../迭代器失效/README.md)中，我们介绍了迭代器失效的规则和安全删除模式

STL 的核心设计理念是**算法与容器分离**：算法不直接操作容器，而是通过迭代器这一通用接口来访问元素。这使得同一套算法可以适用于任何提供合适迭代器的容器，包括自定义容器

## 迭代器是算法与容器之间的桥梁

标准库算法的函数签名几乎都以迭代器作为参数，而非容器本身

```cpp
// std::sort 接受一对迭代器，而非容器
template <typename RandomIt>
void sort(RandomIt first, RandomIt last);

// std::find 同样接受迭代器范围
template <typename InputIt, typename T>
InputIt find(InputIt first, InputIt last, const T& value);
```

这意味着我们可以对任何提供对应迭代器的容器使用这些算法

```cpp
std::vector<int> vec = {3, 1, 4, 1, 5};
std::sort(vec.begin(), vec.end()); // 对 vector 排序

int arr[] = {3, 1, 4, 1, 5};
std::sort(std::begin(arr), std::end(arr)); // 对原生数组排序
```

## 算法对迭代器类别的要求

不同算法对迭代器的能力要求不同，这由算法的操作逻辑决定。如果传入的迭代器类别不满足要求，编译将会报错

| 算法类别 | 要求的迭代器类别 | 原因 | 典型算法 |
| :-----: | :-----------: | :---: | :-----: |
| 只读遍历 | `input_iterator` | 只需逐个读取元素 | `std::find`, `std::count`, `std::accumulate`, `std::equal` |
| 只写遍历 | `output_iterator` | 只需逐个写入元素 | `std::copy`, `std::fill`, `std::transform` (输出) |
| 多趟遍历 | `forward_iterator` | 需要多趟遍历同一序列 | `std::replace`, `std::remove`, `std::unique` |
| 双向遍历 | `bidirectional_iterator` | 需要反向移动 | `std::reverse`, `std::reverse_copy` |
| 随机访问 | `random_access_iterator` | 需要跳跃式访问或排序 | `std::sort`, `std::nth_element`, `std::partial_sort` |

### 示例：`std::sort` 要求随机访问迭代器

```cpp
std::list<int> lst = {3, 1, 4, 1, 5};
std::sort(lst.begin(), lst.end()); // 编译错误！list 的迭代器是双向迭代器
```

报错信息

```bash
/usr/include/c++/16/bits/stl_algo.h:1914:50: error: no match for 'operator-' (operand types are 'std::_List_iterator<int>' and 'std::_List_iterator<int>')
 1914 |                                 std::__lg(__last - __first) * 2,
      |                                           ~~~~~~~^~~~~~~~~
```

`std::list` 的迭代器是双向迭代器，不支持随机访问（如 `it + n`、`it - n`），因此无法用于 `std::sort`。对于 `std::list`，应使用其成员函数 `lst.sort()`

### 示例：`std::reverse` 要求双向迭代器

```cpp
std::vector<int> vec = {1, 2, 3, 4, 5};
std::reverse(vec.begin(), vec.end()); // 正确：vector 提供随机访问迭代器（满足双向要求）

std::forward_list<int> flst = {1, 2, 3, 4, 5};
std::reverse(flst.begin(), flst.end()); // 编译错误！forward_list 的迭代器是前向迭代器
```

## 迭代器类别驱动的算法分派

标准库内部利用 `iterator_category` 对同一算法进行**分派 (dispatch)**，即根据迭代器类别选择不同的实现策略，以获得最优性能

### `std::distance` 的分派示例

- 对于**随机访问迭代器**，直接使用 `last - first`，O(1)
- 对于**其他迭代器**，逐个递增并计数，O(n)

```cpp
// 标准库内部实现（简化）
template <typename InputIt>
typename std::iterator_traits<InputIt>::difference_type
distance(InputIt first, InputIt last)
{
    // 根据 iterator_category 分派到不同实现
    return distance_impl(first, last, typename std::iterator_traits<InputIt>::iterator_category{});
}

// 随机访问迭代器版本：O(1)
template <typename RandomIt>
std::ptrdiff_t distance_impl(RandomIt first, RandomIt last, std::random_access_iterator_tag)
{
    return last - first;
}

// 输入迭代器版本：O(n)
template <typename InputIt>
std::ptrdiff_t distance_impl(InputIt first, InputIt last, std::input_iterator_tag)
{
    std::ptrdiff_t n = 0;
    while (first != last) { ++first; ++n; }
    return n;
}
```

```cpp
std::vector<int> vec = {1, 2, 3, 4, 5};
std::cout << std::distance(vec.begin(), vec.end()) << "\n"; // O(1)，直接做指针减法

std::list<int> lst = {1, 2, 3, 4, 5};
std::cout << std::distance(lst.begin(), lst.end()) << "\n"; // O(n)，逐个遍历计数
```

### `std::advance` 的分派示例

- 对于**随机访问迭代器**，直接 `it += n`，O(1)
- 对于**双向迭代器**，根据 `n` 的正负分别 `++` 或 `--`，O(n)
- 对于**输入/前向迭代器**，只能 `++`，O(n)，且 `n` 必须非负

## 常用算法与迭代器配合示例

### 遍历与查找 — 输入迭代器

```cpp
std::vector<int> vec = {1, 2, 3, 4, 5};

// std::find：查找元素
auto it = std::find(vec.begin(), vec.end(), 3);
if (it != vec.end())
{
    std::cout << "找到: " << *it << "\n"; // 输出: 找到: 3
}

// std::count：统计元素出现次数
int n = std::count(vec.begin(), vec.end(), 1);
std::cout << "1 出现了 " << n << " 次\n"; // 输出: 1 出现了 1 次

// std::accumulate：累加（需要 <numeric>）
int sum = std::accumulate(vec.begin(), vec.end(), 0);
std::cout << "总和: " << sum << "\n"; // 输出: 总和: 15
```

### 修改与变换 — 前向/输出迭代器

```cpp
std::vector<int> vec = {1, 2, 3, 4, 5};

// std::replace：替换元素
std::replace(vec.begin(), vec.end(), 3, 99);
// vec 为 {1, 2, 99, 4, 5}

// std::transform：变换元素，写入到另一个容器
std::vector<int> result(5);
std::transform(vec.begin(), vec.end(), result.begin(), [](int x) { return x * 2; });
// result 为 {2, 4, 198, 8, 10}

// std::copy + back_inserter：复制到动态增长的容器
std::vector<int> dest;
std::copy(vec.begin(), vec.end(), std::back_inserter(dest));
// dest 为 {1, 2, 99, 4, 5}
```

### 排序与重排 — 随机访问迭代器

```cpp
std::vector<int> vec = {3, 1, 4, 1, 5, 9, 2, 6};

// std::sort：排序
std::sort(vec.begin(), vec.end());
// vec 为 {1, 1, 2, 3, 4, 5, 6, 9}

// std::sort 自定义比较
std::sort(vec.begin(), vec.end(), std::greater<int>());
// vec 为 {9, 6, 5, 4, 3, 2, 1, 1}

// std::nth_element：第 n 小的元素放到正确位置
std::vector<int> v = {3, 1, 4, 1, 5, 9, 2, 6};
std::nth_element(v.begin(), v.begin() + 3, v.end());
// v[3] 为 3（第 4 小的元素），其左侧都 ≤ 3，右侧都 ≥ 3

// std::partial_sort：部分排序
std::vector<int> v2 = {3, 1, 4, 1, 5, 9, 2, 6};
std::partial_sort(v2.begin(), v2.begin() + 3, v2.end());
// v2 前 3 个为最小的 3 个元素且有序：{1, 1, 2, ...}
```

### 双向遍历 — 双向迭代器

```cpp
std::list<int> lst = {1, 2, 3, 4, 5};

// std::reverse：反转（要求双向迭代器）
std::reverse(lst.begin(), lst.end());
// lst 为 {5, 4, 3, 2, 1}

// std::list 自身的排序（成员函数，因为 std::sort 要求随机访问迭代器）
lst.sort();
// lst 为 {1, 2, 3, 4, 5}
```

## 迭代器范围与算法

C++20 引入了 **范围 (Ranges)** 库，它将迭代器对 `[first, last)` 封装为统一的范围对象，使算法调用更加简洁

### 传统写法 vs Ranges 写法

```cpp
std::vector<int> vec = {3, 1, 4, 1, 5, 9, 2, 6};

// 传统写法：传入迭代器对
std::sort(vec.begin(), vec.end());
auto it = std::find(vec.begin(), vec.end(), 4);

// C++20 Ranges 写法：传入范围对象
std::ranges::sort(vec);
auto it2 = std::ranges::find(vec, 4);
```

### 管道组合 (Views)

Ranges 的 Views 支持通过管道运算符 `|` 进行惰性组合

```cpp
std::vector<int> vec = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

// 取偶数，平方，取前 3 个
auto result = vec
    | std::views::filter([](int x) { return x % 2 == 0; })
    | std::views::transform([](int x) { return x * x; })
    | std::views::take(3);

// result 为 {4, 16, 36}（惰性求值，不产生中间容器）
for (int x : result)
{
    std::cout << x << " ";
}
std::cout << "\n";
```

## 总结

| 要点 | 说明 |
| :---: | :---: |
| 算法不直接操作容器 | 通过迭代器访问元素，实现算法与容器的解耦 |
| 算法对迭代器有类别要求 | 不满足要求的迭代器会导致编译错误 |
| 迭代器类别驱动算法分派 | 标准库根据 `iterator_category` 选择最优实现 |
| 迭代器适配器扩展算法适用范围 | 如 `back_inserter` 使 `std::copy` 可向空容器写入 |
| C++20 Ranges 简化接口 | 范围对象替代迭代器对，Views 支持惰性组合 |
