# 向量

向量 (Vector) 是 C++ 标准库中的一个容器，用于存储一组元素

相比数组，向量是可动态扩展的，可以自动调整大小

## 语法

要使用向量，需要包含头文件 `<vector>`，并使用 `std::vector` 类型

```cpp
#include <vector>
```

### 创建向量

```cpp
std::vector<类型> 向量名 = {元素1, 元素2, 元素3, ...};
std::vector<类型> 向量名(初始大小, 初始值);
```

- 初始值可以省略，如果省略会有以下行为
    - 对于基本类型会零初始化
    - 对于类类型会调用默认构造函数

## 示例

假设现在有一个向量，我们希望对它进行一些操作

```cpp
using std::vector;

vector<int> vec;
```

### 添加元素

使用 `push_back` 方法向向量中添加元素 (添加到末尾)

```cpp
for (int i = 1; i <= 10; i++)
{
    vec.push_back(i); // 添加 1 到 10
}
```

### 获取向量大小

使用 `size` 方法获取向量的大小

```cpp
int size = vec.size(); // 获取向量大小
for (int i = 0; i < size; i++)
{
    std::cout << vec[i] << " "; // 输出向量元素
}
std::cout << "\n";
```

### 获取向量元素

使用 `at` 方法获取向量的元素

与下标操作符 `[]` 的区别在于，`at` 方法会检查索引是否超出范围，而 `[]` 操作符不会

如果索引超出范围，会抛出 `std::out_of_range` 异常

所以推荐配合 `try-catch` 块使用

```cpp
try
{
    std::cout << vec.at(0) << "\n"; // 输出向量第一个元素
}
catch (const std::out_of_range& e)
{
    std::cout << "索引超出范围" << "\n";
}
```

### 插入元素

使用 `insert` 方法在向量中插入元素

此方法的第一个参数是插入位置的迭代器，第二个参数是要插入的元素

可以使用 `begin()` 获取起始迭代器，配合指针偏移定位插入位置

```cpp
vec.insert(vec.begin() + 5, 100);
```

- `+5` 表示在索引为 5 的位置插入元素
- 由于 `insert` 不会检查索引是否超出范围 (合法插入范围是 `[0, size]`)，所以插入越界是未定义行为

### 删除元素

跟 `insert` 方法类似，都需要使用迭代器

```cpp
vec.erase(vec.begin() + 5);
```

- 跟 `insert` 方法一样，不会检查索引是否超出范围

### 清空向量

使用 `clear` 方法清空向量

```cpp
vec.clear();
```
