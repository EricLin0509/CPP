# 遍历数组

遍历数组用于获取数组中的每个元素，可以用于数组的初始化、遍历、统计等操作

## 示例

假设现在我们有一个字符串数组存储了学生的名字，我们想要输出每个学生的名字

```cpp
std::string students[] = {"Alice", "Bob", "Charlie", "David"};

std::cout << students[0] << std::endl; // 输出 Alice
std::cout << students[1] << std::endl; // 输出 Bob
std::cout << students[2] << std::endl; // 输出 Charlie
std::cout << students[3] << std::endl; // 输出 David
```

但是这样我们只能输出数组中的前四个元素，如果数组的大小是未知的，我们无法知道需要输出多少个元素

### 设计循环

使用循环可以遍历数组中的所有元素，循环的次数是数组的大小

```cpp
for (int i = 0; ; i++)
{
    std::cout << students[i] << std::endl;
}
```

但是这样循环会无限循环下去，因为数组的大小是未知的，我们无法知道需要输出多少个元素

### 确定数组元素个数

我们可以使用 `sizeof` 运算符来获取数组的大小，然后使用循环遍历数组中的所有元素

1. 使用 `sizeof(students)` 获取整个数组的大小
2. 使用 `sizeof(students[0])` 获取数组中每个元素的大小
3. 使用 `sizeof(students) / sizeof(students[0])` 计算数组元素个数

```cpp
for (int i = 0; i < sizeof(students) / sizeof(students[0]); i++)
{
    std::cout << students[i] << std::endl;
}
```
