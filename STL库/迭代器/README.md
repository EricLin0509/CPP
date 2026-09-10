# 迭代器

迭代器是一种广义化的指针，它使得 C++ 程序可以通过统一的方式处理不同的数据结构

## 简单示例

假设现在有一个 `vector`，我们想要遍历它并打印出每个元素

```cpp
std::vector<int> vec = {1, 2, 3, 4, 5};
```

使用 `for` 循环遍历 `vector` 并打印出每个元素

```cpp
for (int i = 0; i < vec.size(); i++)
{
    std::cout << vec[i] << " ";
}
std::cout << "\n";
```

通用可以使用迭代器遍历 `vector` 并打印出每个元素

```cpp
for (std::vector<int>::iterator it = vec.begin(); it != vec.end(); it++)
{
    std::cout << it << " ";
}
std::cout << "\n";
```

此时会报错

```bash
error: no match for ‘operator<<’ (operand types are ‘std::ostream’ {aka ‘std::basic_ostream<char>’} and ‘std::vector<int>::iterator’)
    9 |         std::cout << it << " ";
      |         ~~~~~~~~~ ^~ ~~
      |              |       |
      |              |       std::vector<int>::iterator
      |              std::ostream {aka std::basic_ostream<char>}
```

这是因为迭代器相当于一个包裹了指向某个容器里元素的指针的类，而 `std::cout` 不支持处理 `std::vector<int>::iterator` 类型的参数

为了能够正确地处理迭代器里面的元素，我们需要使用 `*` 运算符来获取迭代器所指向的元素 (即通过迭代器实现的 `operator*()` 操作符)

```cpp
for (std::vector<int>::iterator it = vec.begin(); it != vec.end(); it++)
{
    std::cout << *it << " ";
}
std::cout << "\n";
```

## 迭代器操作

有以下迭代器操作方法

- `next`: 返回一个指向下一个元素的迭代器
- `prev`: 返回一个指向前一个元素的迭代器
- `advance`: 将迭代器向前移动指定的步数
- `distance`: 计算两个迭代器之间的距离

### `std::next()`

```cpp
std::vector<int> vec = {1, 2, 3, 4, 5};
std::vector<int>::iterator it = vec.begin();
it = std::next(it); // 指向 2
```

### `std::prev()`

```cpp
std::vector<int> vec = {1, 2, 3, 4, 5};
std::vector<int>::iterator it = vec.end();
it = std::prev(it); // 指向 5
```

### `std::advance()`

```cpp
std::vector<int> vec = {1, 2, 3, 4, 5};
std::vector<int>::iterator it = vec.begin();
std::advance(it, 2); // 将迭代器向前移动2步，即指向3
```

### `std::distance()`

```cpp
std::vector<int> vec = {1, 2, 3, 4, 5};
std::vector<int>::iterator it1 = vec.begin();
std::vector<int>::iterator it2 = vec.end();
std::cout << std::distance(it1, it2) << "\n"; // 输出5
```

## 迭代器实现详解

我们将会以实现一个斐波那契数列的迭代器，详细讲解迭代器的实现

1. [迭代器特征](迭代器特征/README.md)
2. [迭代器操作](迭代器操作/README.md)
3. 迭代器适配器
4. 迭代器失效
5. 迭代器与算法配合

初始模板类定义如下

```cpp
template <typename T>
class FibonacciIterator {
    private:
        T curr;
        T prev;
    public:
        FibonacciIterator()
        {
            curr = 1;
            prev = 0;
        }
};
```
