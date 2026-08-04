# void类型指针

`void`类型指针可以指向任何类型的变量

但不能直接访问该变量的值或进行指针运算

它更像一种通用的指针，可以用来接收任何类型的指针

## 语法

### 声明

```cpp
void *ptr;
```

### 强制转换

`void` 类型指针支持强制转换成任意类型的指针

```cpp
void *ptr;
int *intPtr = (int *)ptr;

((char *)ptr)++; // 这里先将 ptr 强制转换为 char 类型指针，然后进行指针运算
```

### 不支持的操作

`void` 类型指针不支持**解引用**和**指针运算**

```cpp
void *ptr;
ptr++; // 这里会报错
*ptr; // 这里会报错
```

## 示例

假设现在想实现一个类似 `memcmp` 的函数，用于比较两组数据

```cpp
int memcmp_int(int *ptr1, int *ptr2, size_t count)
{
    for (size_t i = 0; i < count; i++)
    {
        if (ptr1[i] != ptr2[i])
            return ptr1[i] - ptr2[i];
    }

    return 0;
}
```

如果现在要比较两个 `int` 类型数组没有问题

```cpp
int arr1[] = {1, 2, 3};
int arr2[] = {4, 5, 6};
int result = memcmp_int(arr1, arr2, sizeof(arr1) / sizeof(arr1[0]));
```

但如果现在要比较两个 `double` 类型数组

```cpp
double arr1[] = {1.1, 2.2, 3.3};
double arr2[] = {4.4, 5.5, 6.6};
int result = memcmp_int(arr1, arr2, sizeof(arr1) / sizeof(arr1[0]));
```

此时会报错

```
void_pointer.cpp:17:25: error: 不能将‘double*’转换为‘int*’
```

这是因为 C++ 是强类型语言，不能用 `int` 类型指针来接收 `double` 类型的指针

虽说可以设计一个 `memcmp_double` 函数来比较两个 `double` 类型数组

```cpp
double memcmp_double(double *ptr1, double *ptr2, size_t count)
{
    for (size_t i = 0; i < count; i++)
    {
        if (ptr1[i] != ptr2[i])
            return ptr1[i] - ptr2[i];
    }

    return 0;
}
```

但这样如果我们要处理不同类型的数据如 `char`、`short`、`long`、`float`、`double` 甚至是结构体等类型

需要为每个类型都编写一个 `memcmp` 函数，这样效率低且代码冗余

那我们可以使用 `void` 类型指针来解决这个问题

### 使用 `void` 类型指针

为了能实现比较每个字节的数据，需要将 `void` 类型指针强制转换为 `unsigned char` 类型指针

```cpp
int memcmp(void *ptr1, void *ptr2, size_t size)
{
    unsigned char *data1 = (unsigned char *)ptr1;
    unsigned char *data2 = (unsigned char *)ptr2;
    
    for (size_t i = 0; i < size; i++)
    {
        if (data1[i] != data2[i])
            return data1[i] - data2[i];
    }
    
    return 0;
}
```

- 无符号 `unsigned` 类型可以避免符号扩展问题 (如字节值 `0xFF` 不会被解释为 `-1`)
- 参数 `size` 不同与上面的 `count`，`size` 是**字节大小**，而 `count` 是元素个数

这样就可以比较不同类型的数据了

```cpp
int arr1[] = {1, 2, 3};
int arr2[] = {4, 5, 6};
int result_int = memcmp(arr1, arr2, sizeof(arr1));

double arr3[] = {1.1, 2.2, 3.3};
double arr4[] = {4.4, 5.5, 6.6};
int result_double = memcmp(arr3, arr4, sizeof(arr3));
```
