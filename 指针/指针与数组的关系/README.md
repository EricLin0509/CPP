# 指针与数组的关系

这两者的关系是：数组名是一个可以隐式转换为指向数组首元素的指针

## 示例

假设现在有一个数组 `arr`

```cpp
int arr[5] = {1, 2, 3, 4, 5};
```

当我们尝试直接输出 `arr`

```cpp
std::cout << "arr: " << arr << "\n";
```

```
arr: 0x7fffffffd5e0
```

可以看到，直接输出 `arr` 的结果是一个地址，说明 `arr` 可以隐式转换为指针

现在尝试直接解引用 `arr`

```cpp
std::cout << "*arr: " << *arr << "\n";
```

```
*arr: 1
```

可以看到，直接解引用 `arr` 的结果是数组的第一个元素，说明 `arr` 是一个指向数组**首元素**的指针

那么就可以解释索引操作符 `[]` 的工作原理了

### 索引操作符的工作原理

索引操作符 `[]` 的工作原理如下

1. 先将索引操作符的索引值当成一个偏移量，加到数组名上
    - 例如 `arr[2]` 先转换成 `arr + 2`
2. 然后解引用这个地址，得到数组元素的值
    - 例如 `arr[2]` 解引用得到 `*(arr + 2)`

```cpp
std::cout << "arr[2]: " << arr[2] << "\n";
std::cout << "*(arr + 2): " << *(arr + 2) << "\n";
```

```
arr[2]: 3
*(arr + 2): 3
```

由于指针计算满足交换律，所以甚至可以这样写

```cpp
std::cout << "2[arr]: " << 2[arr] << "\n";
```

```
2[arr]: 3
```

这是因为 `2[arr]` 等价于 `*(2 + arr)`，通过交换律可以得到 `*(arr + 2)`，所以结果是相同的

### 数组退化为指针

上面的例子都是将数组名隐式转换为指针

还有一种情况是将数组作为函数参数时会退化为指针

```cpp
void printArray(int arr[]) // 实际等价于 void printArray(int* arr)
{
    std::cout << "Size of arr: " << sizeof(arr) << "\n"; // 此时是指针的大小
    for (int i = 0; i < 5; i++)
    {
        std::cout << arr[i] << " ";
    }
    std::cout << "\n";
}
```

这种写法编译器会警告

```
array_vs_pointer.cpp:5:44: warning: ‘sizeof’ on array function parameter ‘arr’ will return size of ‘int*’ [-Wsizeof-array-argument]
```

这是因为数组名在函数参数中退化为指针，所以不要在数组作为函数参数时使用 `sizeof` 运算符

但某些情况下，数组名不会隐式转换为指针

例如使用 `sizeof` 运算符时，数组名不会隐式转换为指针

```cpp
int arr[5] = {1, 2, 3, 4, 5};
std::cout << "Size of arr: " << sizeof(arr) << "\n";
```

此时 `sizeof(arr)` 的结果是整个数组的大小，而不是指针的大小 (4或8字节)

```
Size of arr: 20
```
