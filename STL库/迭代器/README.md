# 迭代器

迭代器是一种广义化的指针，它使得 C++ 程序可以通过统一的方式处理不同的数据结构

## 迭代器的分类

- `input_iterator`：输入迭代器
- `output_iterator`：输出迭代器
- `forward_iterator`：前向迭代器
- `bidirectional_iterator`：双向迭代器
- `random_access_iterator`：随机访问迭代器

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

### 返回一个指向下一个元素的迭代器

```cpp
std::vector<int> vec = {1, 2, 3, 4, 5};
std::vector<int>::iterator it = vec.begin();
it = std::next(it); // 指向 2
```

### 返回一个指向前一个元素的迭代器

```cpp
std::vector<int> vec = {1, 2, 3, 4, 5};
std::vector<int>::iterator it = vec.end();
it = std::prev(it); // 指向 4
```

### 将迭代器向前移动指定的步数

```cpp
std::vector<int> vec = {1, 2, 3, 4, 5};
std::vector<int>::iterator it = vec.begin();
std::advance(it, 2); // 将迭代器向前移动2步，即指向3
```

### 计算两个迭代器之间的距离

```cpp
std::vector<int> vec = {1, 2, 3, 4, 5};
std::vector<int>::iterator it1 = vec.begin();
std::vector<int>::iterator it2 = vec.end();
std::cout << std::distance(it1, it2) << "\n"; // 输出5
```
