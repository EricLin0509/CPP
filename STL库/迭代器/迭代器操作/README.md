# 迭代器操作

在上一章[迭代器特征](../迭代器特征/README.md)中，我们介绍了迭代器的基本特征

迭代器的操作是指对迭代器进行的操作，包括移动迭代器、访问元素、修改元素等操作

## 不同迭代器支持的操作

不同迭代器类型支持的操作不同，需要根据类中 `iterator_category` 设置来判断

以下为不同迭代器支持的操作

| 迭代器类型 | 读取/写入操作 | 移动操作 | 比较操作 |
| :-------: | :---------: | :----: | :-----: |
| `input_iterator` | 只读 (`it->`, `*it`) | 自增操作 (`++it`, `it++`) | `==`, `!=` |
| `output_iterator` | 只写 (`*it = val`) | 自增操作 (`++it`, `it++`) | 不支持 |
| `forward_iterator` | 读写 (`*it`) | 自增操作 (`++it`, `it++`) | `==`, `!=` |
| `bidirectional_iterator` | 读写 (`*it`) | 自增/自减操作 (`++it`, `--it`) | `==`, `!=` |
| `random_access_iterator` | 读写 (`*it`, `it[n]`) | 自增/自减操作 (`++it`, `--it`) | `==`, `!=`, `<`, `>`, `<=`, `>=` |

- 这是区分迭代器类型的重要依据

## 为迭代器定义操作方法

当前的代码会报错

```bash
stl_iterator_base_funcs.h:92:11: error: no match for 'operator++' (operand type is 'FibonacciIterator<int>')
   92 |           ++__first;
      |           ^~~~~~~~~
```

是因为缺少 `operator++` 操作方法

### 补充必须的操作方法

由于 `FibonacciIterator` 是前向迭代器类型，所以需要实现 `operator*`、`operator++`、`operator!=`、 `operator==` 方法

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

        T operator*() const // 解引用操作符
        {
            return curr;
        }

        FibonacciIterator& operator++() // 前缀自增操作符 ++it
        {
            T temp = curr;
            curr = prev + curr;
            prev = temp;
            return *this;
        }

        FibonacciIterator operator++(int) // 后缀自增操作符 it++
        {
            FibonacciIterator temp = *this;
            ++(*this);
            return temp;
        }

        bool operator!=(const FibonacciIterator& other) const // 不等于操作符
        {
            return this->curr != other.curr;
        }

        bool operator==(const FibonacciIterator& other) const // 等于操作符
        {
            return this->curr == other.curr;
        }
};
```

### 测试迭代器

现在我们想输出斐波那契数列的前10项

```cpp
FibonacciIterator<int> it;
for (int i = 0; i < 10; i++)
{
    std::cout << *it << " ";
    it++;
}
std::cout << "\n";
```

可以正常输出

```bash
1 1 2 3 5 8 13 21 34 55
```
