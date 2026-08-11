# 函数重载

在C++中，函数重载是指在同一个作用域内，定义多个具有相同名称但参数列表不同的函数

## 原理

在编译器处理函数重载时，会根据参数列表生成一个唯一的函数签名，然后在函数调用时，会根据函数签名来选择合适的函数

假设我们定义了这两个函数

```cpp
void print_num(int a);
void print_num(double a);
```

在编译器处理函数重载时，会生成两个不同的函数签名

- _Z9print_numi: `void print_num(int a)`
- _Z9print_numd: `void print_num(double a)`

## 示例

设计一组加法函数，处理不同数量的参数

```cpp
int add(int a, int b)
{
    return a + b;
}

int add(int a, int b, int c)
{
    return a + b + c;
}

int add(int a, int b, int c, int d)
{
    return a + b + c + d;
}
```

```cpp
add(1, 2); // 3
add(1, 2, 3); // 6
add(1, 2, 3, 4); // 10
```

## C语言的模拟实现

C语言 (C99标准之前) 不支持函数重载

但C11引入了 [`_Generic`](https://github.com/EricLin0509/C/blob/main/高级特性/_Generic/README.md) 关键字，它通过泛型选择机制来模拟类似功能

这种方式的局限性包括:
- 需要**手动定义**每个类型的处理函数
- 只能根据**单个表达式**的类型进行分发
- 所有分支的**参数数量和类型必须一致**

```c
int add_int(int a, int b)
{
    return a + b;
}

double add_double(double a, double b)
{
    return a + b;
}

#define add(a, b) _Generic(a, \
    int: add_int(a, b), \
    double: add_double(a, b), \
    default: 0 \
)(a, b)
```

> [!WARNING]
> **注意**: 与C++的自动函数重载不同,C语言的 `_Generic` 需要在编译时明确指定类型映射关系,灵活性较低但性能更好