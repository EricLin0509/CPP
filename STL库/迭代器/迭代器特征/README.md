# 迭代器特征

迭代器特征 (Iterator Traits) 是 C++ 标准库中定义的模板类 `std::iterator_traits`，它提供统一的接口来查询任意迭代器的类型属性

标准库算法（如 `std::distance`、`std::advance`、`std::sort` 等）内部依赖 `std::iterator_traits` 来获取迭代器的类型信息，因此自定义迭代器必须定义这些特征，才能与标准库配合使用

## 5 种必要特征

以下是所有迭代器都必须具有的特征

| 特征 | 描述 | 当 `T = int` 时 |
| :--: | :--: | :-----------: |
| `value_type` | 迭代器所指向元素的类型 | `int` |
| `difference_type` | 表示两个迭代器之间距离的有符号整数类型 | `std::ptrdiff_t` |
| `pointer` | 指向元素的指针类型 | `int*` |
| `reference` | 对元素的引用类型 | `int&` |
| `iterator_category` | 迭代器的类别 | 根据迭代器能力选择 (如 `std::forward_iterator_tag`) |

- 通常情况下，`difference_type` 使用 `std::ptrdiff_t` 即可

## 为迭代器定义特征

前文已经定义了 `FibonacciIterator` 的成员变量，但它目前还无法与标准库算法配合使用

### 缺少特征时的后果

尝试调用 `std::distance()`

```cpp
FibonacciIterator<int> it;
std::distance(it, it);
```

此时会报错

```bash
trait.cpp:13:18: error: no matching function for call to 'distance(FibonacciIterator<int>&, FibonacciIterator<int>&)'
   13 |     std::distance(it, it);
      |     ~~~~~~~~~~~~~^~~~~~~~
```

这是因为 `FibonacciIterator` 缺少 `difference_type` 等特征，`std::iterator_traits` 无法提取到所需的类型信息，导致 `std::distance()` 无法正常工作

### 补充特征定义

注意需要引入 `iterator` 头文件

```cpp
#include <iterator>
```

为 `FibonacciIterator` 补充 5 种特征类型

```cpp
template <typename T>
class FibonacciIterator {
    private:
        T curr;
        T prev;
    public:
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = T*;
        using reference = T&;
        using iterator_category = std::forward_iterator_tag;

        FibonacciIterator()
        {
            curr = 1;
            prev = 0;
        }
};
```

此时再尝试运行代码，依旧会报错

```bash
stl_iterator_base_funcs.h:92:11: error: no match for 'operator++' (operand type is 'FibonacciIterator<int>')
   92 |           ++__first;
      |           ^~~~~~~~~
```

但这已经说明迭代器特征定义生效了——报错原因从"缺少特征"变为了"缺少 `operator++`"，即标准库已经能够识别 `FibonacciIterator` 为前向迭代器，只是尚未实现对应的操作方法

接下来只需实现必要的[操作方法](../迭代器操作/README.md)即可
